//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Holds the CGCClient class
//
//=============================================================================

#include "stdafx.h"
#include "gcclient.h"
#include "steam/isteamgameserver.h"
#include "steam/isteamgamecoordinator.h"
#include "gcsdk_gcmessages.pb.h"
#include "gcsystemmsgs.pb.h"

namespace GCSDK
{

#define SOCDebug(...) Msg( __VA_ARGS__ )
//#define SOCDebug(...) ((void)0)

//------------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------------
CGCClient::CGCClient( bool bGameserver )
: m_pSteamGameCoordinator( NULL ),
	m_memMsg( 0, 1024 ),
#ifndef STEAM
	m_callbackGCMessageAvailable( NULL, NULL ),
	m_CallbackSteamServersDisconnected( NULL, NULL),
	m_CallbackSteamServerConnectFailure( NULL, NULL),
	m_CallbackSteamServersConnected( NULL, NULL ),
#endif
	m_mapSOCache( DefLessFunc( SOID_t ) ),
	m_unVersion( 0 ),
	m_nConnectionStatus( GCConnectionStatus_NO_STEAM ),
	m_bGameserver( bGameserver ),
	m_bSimulateGCConnectionFailure( false ),
	m_nSessionNeed( 0 ),
	m_nLastSessionNeed( -1 ),
	m_bWantSession( true ),
	m_nLauncherType( GCClientLauncherType_DEFAULT ),
	m_usSteamdatagramPort( 0 )
{
#ifndef STEAM
	if( bGameserver )
	{
		m_callbackGCMessageAvailable.SetGameserverFlag();
		m_CallbackSteamServersDisconnected.SetGameserverFlag();
		m_CallbackSteamServerConnectFailure.SetGameserverFlag();
		m_CallbackSteamServersConnected.SetGameserverFlag();
	}
#endif

	if ( CommandLine()->FindParm( "-perfectworld" ) )
	{
		m_nLauncherType = GCClientLauncherType_PERFECTWORLD;
	}

	m_timeLastSendHello.SetLTime( 0 );
	m_timeReceivedConnectionStatus.SetLTime( 0 );
	m_timeLoggedOn.SetLTime( 0 );

	ClearLogonQueueStats();
}


//------------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------------
CGCClient::~CGCClient( )
{
	Uninit();

	FOR_EACH_MAP_FAST( m_mapSOCache, i )
	{
		delete m_mapSOCache[i];
	}
	m_mapSOCache.RemoveAll();
}


//------------------------------------------------------------------------------
// Purpose: Performs the every-frame work required by the GC Client. Mostly that
//			means running yielding jobs.
// Inputs:  ulLimitMicroseconds - The target number of microseconds worth of 
//			work to do this time through the loop.
// Outputs: Returns true if there is still work to do that was skipped because
//			time ran out.
//------------------------------------------------------------------------------
bool CGCClient::BMainLoop( uint64 ulLimitMicroseconds, uint64 ulFrameTimeMicroseconds )
{
	// Don't do any work if not initialized
	if ( !m_pSteamGameCoordinator )
		return false;

	CLimitTimer limitTimer;
	limitTimer.SetLimit( ulLimitMicroseconds );
	CJobTime::UpdateJobTime( ulFrameTimeMicroseconds ? ulFrameTimeMicroseconds : k_cMicroSecPerShellFrame );

	bool bWorkRemaining = m_JobMgr.BFrameFuncRunSleepingJobs( limitTimer );
	bWorkRemaining |= m_JobMgr.BFrameFuncRunYieldingJobs( limitTimer );
	ThinkConnection();
	return bWorkRemaining;
}

void CGCClient::ThinkConnection()
{
	if (m_nConnectionStatus == GCConnectionStatus_HAVE_SESSION || m_nConnectionStatus == GCConnectionStatus_NO_STEAM)
		return;

	int nTimeout;
	if (!m_bWantSession)
	{
		nTimeout = -1;
	}
	else
	{
		if (m_nConnectionStatus == GCConnectionStatus_NO_SESSION_IN_LOGON_QUEUE)
		{
			nTimeout = 90;
		}
		else
		{
			nTimeout = 10;
		}
	}

	uint64 nTimeout2 = 0;
	if (m_nLastSessionNeed != m_nSessionNeed)
	{
		nTimeout2 = 2 * k_nMillion;
	}
	else if (nTimeout > 0)
	{
		nTimeout2 = nTimeout * k_nMillion;
	}
	else
	{
		return;
	}

	if (m_timeLastSendHello.LTime() == 0)
	{
		SendHello();
	}
	else
	{
		uint64 nTime = max(m_timeLastSendHello.LTime(), m_timeReceivedConnectionStatus.LTime());
		if (nTime + nTimeout2 < CJobTime::LJobTimeCur())
		{
			SendHello();
		}
	}
}

void CGCClient::SetSessionNeed( uint32 nSessionNeed, bool bWantSession )
{
	m_nSessionNeed = nSessionNeed;
	m_bWantSession = bWantSession;
	if ( m_nLastSessionNeed == -1 && !bWantSession )
	{
		m_nLastSessionNeed = nSessionNeed;
	}
	else if ( m_nLastSessionNeed != nSessionNeed )
	{
		ThinkConnection();
	}
}

int CGCClient::GetLogonQueueEstimatedSecondsRemaining() const
{
	int result; // eax
	__int64 v3; // rcx

	result = -1;
	if (!m_timeLogonQueueEstimatedTimeExitQueue.LTime())
		return result;
	v3 = m_timeLogonQueueEstimatedTimeExitQueue.LTime() - CJobTime::LJobTimeCur();
	if (v3 >= 0)
		return (int)v3 / k_nMillion;
	return result;
}

int CGCClient::GetLogonQueueApproxWaitSeconds() const
{
	int result; // eax

	result = -1;
	if (!m_timeLogonQueueApproxTimeEnteredQueue.LTime())
		return result;
	if (CJobTime::LJobTimeCur() - m_timeLogonQueueApproxTimeEnteredQueue.LTime() >= 0)
		return ((int)CJobTime::LJobTimeCur() - (int)m_timeLogonQueueApproxTimeEnteredQueue.LTime()) / 1000000;
	return result;
}

//------------------------------------------------------------------------------
// Purpose: Sends a message to the GC
// Inputs:  unMsgType - the type ID of the message to send
//			pubData - The data for the message we're sending
//			cubData - The number of bytes of data in this message including any
//				variable-lengthed data.
// Outputs: Returns false if the send failed. A return value of true doesn't 
//			mean that the message was necessarily received by the GC just that
//			it didn't fail in obvious ways on the client.
//------------------------------------------------------------------------------
bool CGCClient::BSendMessage( uint32 unMsgType, const uint8 *pubData, uint32 cubData )
{
	if( m_nConnectionStatus == GCConnectionStatus_GC_GOING_DOWN )
	{
		EmitWarning( SPEW_NETWORK, 1, "Trying to send message type %d before GC connection established.  This is guaranteed to fail.\n", unMsgType );
		return false;
	}
	else if( m_pSteamGameCoordinator )
	{
		if( m_bSimulateGCConnectionFailure )
		{
			return true;
		}
		else
		{
			g_theMessageList.TallySendMessage( unMsgType, cubData );
			return m_pSteamGameCoordinator->SendMessage( unMsgType, pubData, cubData ) == k_EGCResultOK;
		}
	}
	else
	{
		return false;
	}
}


//------------------------------------------------------------------------------
// Purpose: Sends a message to the GC
// Inputs:  msg		- The message to send
// Outputs: Returns false if the send failed. A return value of true doesn't 
//			mean that the message was necessarily received by the GC just that
//			it didn't fail in obvious ways on the client.
//------------------------------------------------------------------------------
bool CGCClient::BSendMessage( const CGCMsgBase& msg )
{
	return BSendMessage( msg.Hdr().m_eMsg, msg.PubPkt() + sizeof(GCMsgHdr_t), msg.CubPkt() - sizeof(GCMsgHdr_t) );	
}


//-----------------------------------------------------------------------------
// Purpose: Used to send protobuf messages to the GC
//-----------------------------------------------------------------------------
class CProtoBufGCClientSendHandler : public CProtoBufMsgBase::IProtoBufSendHandler
{
public:
	CProtoBufGCClientSendHandler( CGCClient *pGCClient ) 
		: m_pClient( pGCClient ) {}
	virtual bool BAsyncSend( MsgType_t eMsg, const uint8 *pubMsgBytes, uint32 cubSize ) 
	{	
		g_theMessageList.TallySendMessage( eMsg & ~k_EMsgProtoBufFlag, cubSize );
		VPROF_BUDGET( "CGCClient", VPROF_BUDGETGROUP_STEAM );
		{
			VPROF_BUDGET( "CGCClient - BSendGCMsgToClient (ProtoBuf)", VPROF_BUDGETGROUP_STEAM );
			return m_pClient->BSendMessage( eMsg | k_EMsgProtoBufFlag, pubMsgBytes, cubSize );
		}
	}

private:
	CGCClient *m_pClient;
};


//-----------------------------------------------------------------------------
// Purpose: Sends a message to the given SteamID
//-----------------------------------------------------------------------------
bool CGCClient::BSendMessage( const CProtoBufMsgBase& msg )
{
	CProtoBufGCClientSendHandler sender( this );
	return msg.BAsyncSend( sender );
}


//------------------------------------------------------------------------------
// Purpose: Callback handler for the GCMessageAvailable_t callback. Handles 
//			incoming messages.
// Inputs:	pCallback - the callback from Steam
//------------------------------------------------------------------------------
void CGCClient::OnGCMessageAvailable( GCMessageAvailable_t *pCallback )
{
	uint32 cubData;
	uint32 unMsgType;
	while( m_pSteamGameCoordinator && m_pSteamGameCoordinator->IsMessageAvailable( &cubData ) )
	{
		// Get the size of the full message. sizeof( GCMsgHdr_t ) was not sent in the binary data
		uint32 unFullSize = cubData + sizeof( GCMsgHdr_t );
		m_memMsg.EnsureCapacity( unFullSize );
		uint8 *pFullPacket = m_memMsg.Base();
		uint8 *pPacketFromGC = pFullPacket+sizeof(GCMsgHdr_t);

		EGCResults eResult = m_pSteamGameCoordinator->RetrieveMessage( &unMsgType, pPacketFromGC, m_memMsg.Count() - sizeof( GCMsgHdr_t ), &cubData );
		Assert( eResult == k_EGCResultOK );
		if( !m_bSimulateGCConnectionFailure && eResult == k_EGCResultOK )
		{
			if( unMsgType & k_EMsgProtoBufFlag )
			{
				CNetPacket *pGCPacket = CNetPacketPool::AllocNetPacket();
				pGCPacket->Init( cubData, pPacketFromGC );
				CIMsgNetPacketAutoRelease pMsgNetPacket( pGCPacket );

				// Safety check against malformed packet
				if ( pMsgNetPacket.Get() != NULL )
				{
					// dispatch the packet
					DispatchPacket( pMsgNetPacket.Get() );
				}

				// release the packet
				pGCPacket->Release();
			}
			else
			{
				Assert( 0 == (unMsgType & k_EMsgProtoBufFlag ) );

				// get the header so we can fix it up
				GCMsgHdrEx_t *pHdr = (GCMsgHdrEx_t *)pFullPacket;
				pHdr->m_eMsg = unMsgType;
				pHdr->m_ulSteamID = CSteamID().ConvertToUint64();

				// make a new packet for the message so we can dispatch it
				// The CNetPacket takes ownership of the buffer allocated above
				CNetPacket *pGCPacket = CNetPacketPool::AllocNetPacket();
				pGCPacket->Init( unFullSize, pFullPacket );
				CIMsgNetPacketAutoRelease pMsgNetPacket( pGCPacket );

				// Safety check against malformed packet
				if ( pMsgNetPacket.Get() != NULL )
				{

					// dispatch the packet
					DispatchPacket( pMsgNetPacket.Get() );
				}

				// release the packet
				pGCPacket->Release();
			}
		}
	}
}

void CGCClient::ClearLogonQueueStats()
{
	m_nLogonQueuePosition = -1;
	m_nLogonQueueSize = -1;
	m_timeLogonQueueEstimatedTimeExitQueue.SetLTime(0);
	m_timeLogonQueueApproxTimeEnteredQueue.SetLTime(0);
}

void CGCClient::UpdateLogonState()
{
	bool bLoggedOn = false;

	if (m_pSteamUser)
	{
		bLoggedOn = m_pSteamUser->BLoggedOn();
	}
	else if (m_pSteamGameserver)
	{
		bLoggedOn = m_pSteamGameserver->BLoggedOn();
	}

	if (!bLoggedOn)
	{
		m_timeLoggedOn.SetLTime(0);
		m_nConnectionStatus = GCConnectionStatus_NO_STEAM;
		ClearLogonQueueStats();
	}
	else if (m_nConnectionStatus == GCConnectionStatus_NO_STEAM)
	{
		m_timeLoggedOn.SetToJobTime();
		m_nConnectionStatus = GCConnectionStatus_NO_SESSION;
		ClearLogonQueueStats();
	}
}

void CGCClient::OnSteamServersDisconnected( SteamServersDisconnected_t *pParam )
{
	UpdateLogonState();
}

void CGCClient::OnSteamServerConnectFailure( SteamServerConnectFailure_t *pParam )
{
	UpdateLogonState();
}

void CGCClient::OnSteamServersConnected( SteamServersConnected_t *pParam )
{
	UpdateLogonState();
}

//------------------------------------------------------------------------------
// Purpose: Performs all the initialization for the GC Client instance
// Outputs: Returns false if the initialization failed
//------------------------------------------------------------------------------
bool CGCClient::BInit( uint32 unVersion, ISteamClient *pSteamClient, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
	if ( !pSteamClient )
		return false;

	m_pSteamGameCoordinator = (ISteamGameCoordinator*)pSteamClient->GetISteamGenericInterface( hSteamUser, hSteamPipe, STEAMGAMECOORDINATOR_INTERFACE_VERSION );
	if ( !m_pSteamGameCoordinator )
		return false;

	if ( m_bGameserver )
	{
		m_pSteamGameserver = pSteamClient->GetISteamGameServer( hSteamUser, hSteamPipe, STEAMGAMESERVER_INTERFACE_VERSION );
		if ( !m_pSteamGameserver )
			return false;
	}
	else
	{
		m_pSteamUser = pSteamClient->GetISteamUser( hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION );
		if ( !m_pSteamUser )
			return false;
	}

	m_unVersion = unVersion;

	// Set the job pool size. Threads get lazily created so if no code
	// is using the thread pool, no threads will be created.
	m_JobMgr.SetThreadPoolSize( GetCPUInformation().m_nLogicalProcessors - 1 );

	MsgRegistrationFromEnumDescriptor( EGCSystemMsg_descriptor(), GCSDK::MT_GC );
	MsgRegistrationFromEnumDescriptor( EGCBaseClientMsg_descriptor(), GCSDK::MT_GC );

#ifndef STEAM
	m_callbackGCMessageAvailable.Register( this, &CGCClient::OnGCMessageAvailable );
	m_CallbackSteamServersDisconnected.Register( this, &CGCClient::OnSteamServersDisconnected );
	m_CallbackSteamServerConnectFailure.Register( this, &CGCClient::OnSteamServerConnectFailure );
	m_CallbackSteamServersConnected.Register( this, &CGCClient::OnSteamServersConnected );
#endif

	UpdateLogonState();
	OnGCMessageAvailable( NULL );
	ThinkConnection();
	
	return true;
}


//------------------------------------------------------------------------------
// Purpose: Performs all the uninitialization for the GC Client instance
//------------------------------------------------------------------------------
void CGCClient::Uninit( )
{
#ifndef STEAM
	m_callbackGCMessageAvailable.Unregister();
#endif
	m_pSteamGameCoordinator = NULL;
	m_pSteamGameserver = NULL;
	m_pSteamUser = NULL;

	// Clear and remove the SO caches
	unsigned short nMapIndex = m_mapSOCache.FirstInorder();
	while ( m_mapSOCache.IsValidIndex( nMapIndex ) )
	{
		unsigned short nNextMapIndex = m_mapSOCache.NextInorder( nMapIndex );

		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nMapIndex];
		Assert( pSOCache );
		if ( pSOCache )
		{
			// Send notifications, but only if we were actually subscribed
			if ( pSOCache->BIsSubscribed() )
			{
				FOR_EACH_VEC( m_vecListeners, i )
				{
					ISharedObjectListener *pListener = m_vecListeners[ i ];
					pListener->SOCacheUnsubscribed( pSOCache->GetOwner(), eSOCacheEvent_ListenerRemoved );
				}
			}

			// Delete the entry
			delete pSOCache;
			m_mapSOCache.RemoveAt( nMapIndex );
		}

		nMapIndex = nNextMapIndex;
	}
}


//------------------------------------------------------------------------------
// Purpose: Finds the SO cache for this steam ID. If bCreateIfMissing is false,
//			NULL will be returned if the cache can't be found
//------------------------------------------------------------------------------
CGCClientSharedObjectCache *CGCClient::FindSOCache( SOID_t ID, bool bCreateIfMissing )
{
	CUtlMap< CSteamID, CGCClientSharedObjectCache * >::IndexType_t nCache = m_mapSOCache.Find( ID );
	if( m_mapSOCache.IsValidIndex( nCache ) )
		return m_mapSOCache[nCache];
	else
	{
		if( bCreateIfMissing )
		{
			Assert( ID.IsValid() );
			CGCClientSharedObjectCache *pCache = new CGCClientSharedObjectCache( ID );
			m_mapSOCache.Insert( ID, pCache );
			return pCache;
		}
		else
		{
			return NULL;
		}
	}
}

//------------------------------------------------------------------------------
// Purpose: Add a listener to the SO cache, creating it if necessary
//------------------------------------------------------------------------------
bool CGCClient::AddSOCacheListener( ISharedObjectListener *pListener )
{
	if ( m_vecListeners.HasElement( pListener ) )
		return false;

	FOR_EACH_MAP( m_mapSOCache, nMapIndex )
	{
		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nMapIndex];
		Assert( pSOCache );
		if ( pSOCache )
		{
			// Send notifications, but only if we were actually subscribed
			if ( pSOCache->BIsSubscribed() )
			{
				pSOCache->NotifyCreated( *pListener );
			}
		}
	}

	m_vecListeners.AddToTail( pListener );

	return true;
}

//------------------------------------------------------------------------------
// Purpose: Remove listener from the SO cache, if he is listening
//------------------------------------------------------------------------------
bool CGCClient::RemoveSOCacheListener( ISharedObjectListener *pListener )
{
	if ( m_vecListeners.FindAndRemove( pListener ) )
		return false;

	FOR_EACH_MAP( m_mapSOCache, nMapIndex )
	{
		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nMapIndex];
		Assert( pSOCache );
		if ( pSOCache )
		{
			// Send notifications, but only if we were actually subscribed
			if ( pSOCache->BIsSubscribed() )
			{
				pListener->SOCacheUnsubscribed( pSOCache->GetOwner(), eSOCacheEvent_ListenerRemoved );
			}
		}
	}
	
	return true;
}

void CGCClient::DispatchSOCreated( SOID_t owner, const CSharedObject *pObject, ESOCacheEvent eEvent )
{
	FOR_EACH_VEC( m_vecListeners, nListener )
	{
		m_vecListeners[nListener]->SOCreated( owner, pObject, eEvent );
	}
}

void CGCClient::DispatchSOUpdated( SOID_t owner, const CSharedObject *pObject, ESOCacheEvent eEvent )
{
	FOR_EACH_VEC( m_vecListeners, nListener )
	{
		m_vecListeners[nListener]->SOUpdated( owner, pObject, eEvent );
	}
}

void CGCClient::DispatchSODestroyed( SOID_t owner, const CSharedObject *pObject, ESOCacheEvent eEvent )
{
	FOR_EACH_VEC( m_vecListeners, nListener )
	{
		m_vecListeners[nListener]->SODestroyed( owner, pObject, eEvent );
	}
}

void CGCClient::DispatchSOCacheSubscribed( SOID_t owner, ESOCacheEvent eEvent )
{
	FOR_EACH_VEC( m_vecListeners, nListener )
	{
		m_vecListeners[nListener]->SOCacheSubscribed( owner, eEvent );
	}
}

void CGCClient::DispatchSOCacheUnsubscribed( SOID_t owner, ESOCacheEvent eEvent )
{
	FOR_EACH_VEC( m_vecListeners, nListener )
	{
		m_vecListeners[nListener]->SOCacheUnsubscribed( owner, eEvent );
	}
}


//------------------------------------------------------------------------------
// Purpose: Notify that the given SO cache has been unsubscribed
//------------------------------------------------------------------------------
void CGCClient::NotifySOCacheUnsubscribed( SOID_t ID )
{

	CUtlMap< CSteamID, CGCClientSharedObjectCache * >::IndexType_t nCache = m_mapSOCache.Find( ID );
	if( m_mapSOCache.IsValidIndex( nCache ) )
	{

		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nCache];

		// Ignore requests to remove caches that were never subscribed
		if ( pSOCache->BIsSubscribed() )
		{
			SOCDebug( "NotifySOCacheUnsubscribed(%s) [in cache, subscribed]\n", SOIDRender_t( ID ).String()  );
			pSOCache->SetSubscribed( false );
			FOR_EACH_VEC( m_vecListeners, i )
			{
				ISharedObjectListener *pListener = m_vecListeners[ i ];
				pListener->SOCacheUnsubscribed( pSOCache->GetOwner(), eSOCacheEvent_Unsubscribed );
			}
		}
		else
		{
			SOCDebug( "NotifySOCacheUnsubscribed(%s) [in cache, not subscribed]\n", SOIDRender_t( ID ).String()  );
		}

		if ( ID.Type() != k_SOID_Type_SteamID )
		{
			if ( pSOCache )
				delete pSOCache;

			m_mapSOCache.RemoveAt( nCache );
		}
	}
	else
	{
		SOCDebug( "NotifySOCacheUnsubscribed(%s) [not in cache]\n", SOIDRender_t( ID ).String()  );
	}
}

//------------------------------------------------------------------------------
// Purpose: Notify that the given SO cache has been unsubscribed
//------------------------------------------------------------------------------
void CGCClient::NotifyResubscribedUpToDate( SOID_t ID )
{

	CUtlMap< CSteamID, CGCClientSharedObjectCache * >::IndexType_t nCache = m_mapSOCache.Find( ID );
	if( m_mapSOCache.IsValidIndex( nCache ) )
	{

		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nCache];

		if ( !pSOCache->BIsSubscribed() )
		{
			SOCDebug( "NotifyResubscribedUpToDate(%s) [in cache, not subscribed]\n", SOIDRender_t( ID ).String()  );
			pSOCache->SetSubscribed( true );
			FOR_EACH_VEC( m_vecListeners, i )
			{
				ISharedObjectListener *pListener = m_vecListeners[ i ];
				pListener->SOCacheSubscribed( pSOCache->GetOwner(), eSOCacheEvent_Subscribed );
			}
		}
		else
		{
			SOCDebug( "NotifyResubscribedUpToDate(%s) [in cache, subscribed]\n", SOIDRender_t( ID ).String()  );
		}

		if ( ID.Type() != k_SOID_Type_SteamID )
		{
			if ( pSOCache )
				delete pSOCache;

			m_mapSOCache.RemoveAt( nCache );
		}
	}
	else
	{
		SOCDebug( "NotifyResubscribedUpToDate(%s) [not in cache]\n", SOIDRender_t( ID ).String()  );
	}
}

void CGCClient::ProcessSOCacheSubscribedMsg( const CMsgSOCacheSubscribed &msg )
{
	if ( msg.has_owner_soid() )
	{
		CGCClientSharedObjectCache *pSOCache = FindSOCache( msg.owner_soid() );

		Assert( pSOCache );
		if( pSOCache )
		{
			SOCDebug( "ProcessSOCacheSubscribedMsg(owner=%s) [in cache]\n", SOIDRender_t( msg.owner_soid() ).String() );
			DbgVerify( pSOCache->BParseCacheSubscribedMsg( *this, msg ) );
		}
		else
		{
			SOCDebug( "ProcessSOCacheSubscribedMsg(owner=%s) [not in cache]\n", SOIDRender_t( msg.owner_soid() ).String() );
		}

		Test_CacheSubscribed( pSOCache->GetOwner() );
	}
}

void CGCClient::ProcessCacheSubscriptionCheckMsg( const CMsgSOCacheSubscriptionCheck &msg )
{
	if ( msg.has_owner_soid() )
	{
		SOID_t ownerID = msg.owner_soid();

		CGCClientSharedObjectCache *pSOCache = FindSOCache( ownerID, false );

		// if we do not have the cache or it is out-of-date, request a refresh
		if ( pSOCache == NULL || !pSOCache->BIsInitialized() || pSOCache->GetVersion() != msg.version() )
		{
			SOCDebug( "ProcessCacheSubscriptionCheckMsg(owner=%s) -- need refresh\n", SOIDRender_t( msg.owner_soid() ).String() );
			CProtoBufMsg< CMsgSOCacheSubscriptionRefresh > msg_response( k_ESOMsg_CacheSubscriptionRefresh );
			ownerID.ToMsgSOIDOwner( msg_response.Body().mutable_owner_soid() );
			BSendMessage( msg_response );
		}
		else
		{
			SOCDebug( "ProcessCacheSubscriptionCheckMsg(owner=%s) -- up-to-date, no refresh needed\n", SOIDRender_t( msg.owner_soid() ).String() );

			// This is one method by which the GC notifies us that we are subscribed.
			if ( !pSOCache->BIsSubscribed() )
			{
				NotifyResubscribedUpToDate( pSOCache->GetOwner() );
				Assert( pSOCache->BIsSubscribed() );
			}
		}
	}
}

//------------------------------------------------------------------------------
// Purpose: Finds the shared object for this steam ID and key object
//------------------------------------------------------------------------------
CSharedObject *CGCClient::FindSharedObject( SOID_t ID, const CSharedObject & soIndex ) 
{ 
	CGCClientSharedObjectCache *pCache = FindSOCache( ID, false );
	if( pCache )
		return pCache->FindSharedObject( soIndex ); 
	else
		return NULL;
}

void CGCClient::SendHello()
{
	if ( !m_bWantSession )
	{
		EmitInfo( SPEW_GC, 1, 1, "Will not attempt to establish session with GC.\n" );
		return;
	}

	m_timeLastSendHello.SetToJobTime();
	m_nLastSessionNeed = m_nSessionNeed;

	bool bSuccess;
	if ( !m_bGameserver )
	{
		CProtoBufMsg<CMsgClientHello> msg( m_nLauncherType == GCClientLauncherType_PERFECTWORLD ? k_EMsgGCClientHelloPW : k_EMsgGCClientHello );
		msg.Body().set_version( m_unVersion );
		msg.Body().set_client_session_need( m_nSessionNeed );
		msg.Body().set_client_launcher( m_nLauncherType );

		FOR_EACH_MAP( m_mapSOCache, nMapIndex )
		{
			CMsgSOCacheHaveVersion *pMsg = msg.Body().add_socache_have_versions();

			SOID_t soid( m_mapSOCache[nMapIndex]->GetOwner() );
			soid.ToMsgSOIDOwner( pMsg->mutable_soid() );

			pMsg->set_version( m_mapSOCache[nMapIndex]->GetVersion() );
		}

		bSuccess = BSendMessage(msg);
	}
	else
	{
		CProtoBufMsg<CMsgServerHello> msg( k_EMsgGCServerHello );
		msg.Body().set_version( m_unVersion );
		msg.Body().set_steamdatagram_port( m_usSteamdatagramPort );
		msg.Body().set_client_launcher( m_nLauncherType );

		FOR_EACH_MAP( m_mapSOCache, nMapIndex )
		{
			CMsgSOCacheHaveVersion *pMsg = msg.Body().add_socache_have_versions();

			SOID_t soid( m_mapSOCache[nMapIndex]->GetOwner() );
			soid.ToMsgSOIDOwner( pMsg->mutable_soid() );

			pMsg->set_version( m_mapSOCache[nMapIndex]->GetVersion() );
		}

		bSuccess = BSendMessage( msg );
	}

	if ( !bSuccess )
	{
		EmitInfo( SPEW_GC, 1, 1, "Failed to send Hello message to the GC.\n" );
	}
}

void CGCClient::ProcessWelcomeMsg( const CMsgClientWelcome &msg )
{
	MapSOCache_t map( DefLessFunc( SOID_t ) );

	for ( int i = 0; i < msg.uptodate_subscribed_caches_size(); i++ )
	{
		const CMsgSOIDOwner &ownerID = msg.uptodate_subscribed_caches().Get(i).owner_soid();
		if ( map.Find( ownerID ) != map.InvalidIndex() )
			EmitWarning( SPEW_NETWORK, 1, "Welcome message contained %s twice\n", SOIDRender_t( ownerID ).String() );
		else
			map.Insert( ownerID );
		ProcessCacheSubscriptionCheckMsg( msg.uptodate_subscribed_caches().Get(i) );
	}

	for ( int i = 0; i < msg.outofdate_subscribed_caches_size(); i++ )
	{
		const CMsgSOIDOwner &ownerID = msg.outofdate_subscribed_caches().Get(i).owner_soid();
		if ( map.Find( ownerID ) != map.InvalidIndex() )
			EmitWarning( SPEW_NETWORK, 1, "Welcome message contained %s twice\n", SOIDRender_t( ownerID ).String() );
		else
			map.Insert( ownerID );
		ProcessSOCacheSubscribedMsg( msg.outofdate_subscribed_caches().Get(i) );
	}

	CUtlVector<SOID_t> vec;

	FOR_EACH_MAP( m_mapSOCache, i )
	{
		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[i];
		if ( map.Find( pSOCache->GetOwner() ) == map.InvalidIndex() )
		{
			vec.AddToTail( pSOCache->GetOwner() );
		}
	}

	FOR_EACH_VEC( vec, i )
	{
		NotifySOCacheUnsubscribed( vec[i] );
	}
}

void CGCClient::SetSimulateGCConnectionFailure( bool bForcedFailure )
{
	m_bSimulateGCConnectionFailure = bForcedFailure;

	if ( bForcedFailure )
	{
		EmitInfo( SPEW_GC, 1, LOG_ALWAYS, "Simulated GC communications failure is active.\n" );
		if ( m_nConnectionStatus != GCConnectionStatus_NO_STEAM )
			m_nConnectionStatus = GCConnectionStatus_NO_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
	}
	else
	{
		EmitInfo( SPEW_GC, 1, LOG_ALWAYS, "Simulated GC communications failure is not active.\n" );
	}
}

void CGCClient::MessageReplyTimedOut( uint32 nExpectedMsg, uint nTimeoutSecs )
{
	if ( m_nConnectionStatus == GCConnectionStatus_HAVE_SESSION )
	{
		EmitWarning( SPEW_NETWORK, 1, "Timed out waiting %d secs for reply msg %d from GC.  Assuming connection has been disrupted.\n", nTimeoutSecs, nExpectedMsg );
		m_nConnectionStatus = GCConnectionStatus_NO_SESSION;
		ThinkConnection();
	}
}

void CGCClient::DispatchPacket( IMsgNetPacket *pMsgNetPacket )
{
	MsgType_t eMsg = pMsgNetPacket->GetEMsg();

	if ( eMsg == k_EMsgGCClientConnectionStatus || eMsg == k_EMsgGCServerConnectionStatus )
	{
		CProtoBufMsg< CMsgConnectionStatus > msg( pMsgNetPacket );
		
		uint32 nPrevConnectionStatus = m_nConnectionStatus;
		m_nConnectionStatus = msg.Body().status();
		m_timeReceivedConnectionStatus.SetToJobTime();
		ClearLogonQueueStats();

		if ( msg.Body().has_queue_position() && msg.Body().has_queue_size() )
		{
			m_nLogonQueuePosition = msg.Body().queue_position();
			m_nLogonQueueSize = msg.Body().queue_size();
			Msg( "You are #%d in line of %d waiting players.\n", m_nLogonQueuePosition, m_nLogonQueueSize );
		}

		if ( msg.Body().has_wait_seconds() )
		{
			int wait_seconds = msg.Body().wait_seconds();
			Msg( "You have been waiting for approximately %02d:%02d.\n", wait_seconds / 60, wait_seconds % 60 );
			m_timeLogonQueueApproxTimeEnteredQueue.SetFromJobTime( wait_seconds * -k_nMillion );
		}

		int estimated_wait_seconds_remaining = msg.Body().estimated_wait_seconds_remaining();
		if ( estimated_wait_seconds_remaining > 0 )
		{
			Msg( "Estimated wait time remaining is %02d:%02d.\n", estimated_wait_seconds_remaining / 60, estimated_wait_seconds_remaining % 60 );
			m_timeLogonQueueEstimatedTimeExitQueue.SetFromJobTime( estimated_wait_seconds_remaining * k_nMillion );
		}

		if ( m_nConnectionStatus == GCConnectionStatus_HAVE_SESSION )
		{
			EmitInfo( SPEW_GC, 1, 1, "Connection to GC confirmed.\n" );
		}
		else if ( m_nConnectionStatus == GCConnectionStatus_NO_SESSION )
		{
			if ( nPrevConnectionStatus == GCConnectionStatus_HAVE_SESSION )
				EmitInfo( SPEW_GC, 1, 1, "Connection to GC has been lost.\n" );
			if ( m_bWantSession )
				EmitInfo( SPEW_GC, 1, 1, "Attempting to re-establish GC connection.\n" );
			else
				EmitInfo( SPEW_GC, 1, 1, "Notifying GC we don't need a session.\n" );
			
			SendHello();
		}
		else if ( m_nConnectionStatus == GCConnectionStatus_NO_SESSION_IN_LOGON_QUEUE )
		{
			EmitInfo( SPEW_GC, 1, 1, "In logon queue; waiting for GC to confirm connection.\n" );
			m_nLastSessionNeed = msg.Body().client_session_need();
		}
		else
		{
			m_timeLastSendHello.SetToJobTime();
			float flRandom = ( RandomFloat( 0, 10 ) + 60 ) * k_nMillion;
			m_timeLastSendHello += Clamp( (int)flRandom, 0, INT32_MAX );
		}
	}
	if ( eMsg == k_EMsgGCServerWelcome && m_bGameserver )
	{
		EmitInfo( SPEW_GC, 1, 1, "Received server welcome from GC.\n" );
		
		m_nConnectionStatus = GCConnectionStatus_HAVE_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
		ClearLogonQueueStats();

		CProtoBufMsg< CMsgClientWelcome > msg( pMsgNetPacket );
		ProcessWelcomeMsg( msg.Body() );
	}
	if ( eMsg == k_EMsgGCClientWelcome && !m_bGameserver )
	{
		EmitInfo( SPEW_GC, 1, 1, "Received client welcome from GC.\n" );
		
		m_nConnectionStatus = GCConnectionStatus_HAVE_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
		ClearLogonQueueStats();

		CProtoBufMsg< CMsgClientWelcome > msg( pMsgNetPacket );
		ProcessWelcomeMsg( msg.Body() );
	}

	// dispatch the packet
	GetJobMgr().BRouteMsgToJob( this, pMsgNetPacket, JobMsgInfo_t( pMsgNetPacket->GetEMsg(), pMsgNetPacket->GetSourceJobID(), pMsgNetPacket->GetTargetJobID() ), k_eGCMsgContext_All );

	// keep track of how much we've sent/received this message
	g_theMessageList.TallySendMessage( pMsgNetPacket->GetEMsg(), pMsgNetPacket->CubData() );
}


//------------------------------------------------------------------------------
// Purpose: Validates all the statics in the GCSDKLib that need to be validated
//			when linked directly into the steam servers.
//------------------------------------------------------------------------------
#ifdef DBGFLAG_VALIDATE
void CGCClient::ValidateStatics( CValidator &validator )
{
	// Validate the global message list
	g_theMessageList.Validate( validator, "g_theMessageList" );

	// Validate the network global memory pool
	g_MemPoolMsg.Validate( validator, "g_MemPoolMsg" );

	CNetPacketPool::ValidateGlobals( validator );

	CJobMgr::ValidateStatics( validator, "CJobMgr" );
	CJob::ValidateStatics( validator, "CJob" );
	ValidateTempTextBuffers( validator );
	CSharedObject::ValidateStatics( validator );

	// validate the SQL access layer
	CRecordBase::ValidateStatics( validator, "CRecordBase" );
	GSchemaFull().Validate( validator, "GSchemaFull" );
	CRecordInfo::ValidateStatics( validator, "CRecordInfo" );
}
#endif // DBGFLAG_VALIDATE


class CGCSOCreateJob : public CGCClientJob
{
public:
	CGCSOCreateJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOSingleObject> msg( pNetPacket );
		SOCDebug( "CGCSOCreateJob(owner=%s, type=%d)\n", SOIDRender_t( msg.Body().owner_soid() ).String(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( msg.Body().owner_soid() );
		if ( pSOCache )
		{
			pSOCache->BCreateFromMsg( *m_pGCClient, msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCreateJob, "CGCSOCreateJob", k_ESOMsg_Create );

class CGCSODestroyJob : public CGCClientJob
{
public:
	CGCSODestroyJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOSingleObject> msg( pNetPacket );
		SOCDebug( "CGCSODestroyJob(owner=%s, type=%d)\n", SOIDRender_t( msg.Body().owner_soid() ).String(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pCache = m_pGCClient->FindSOCache( msg.Body().owner_soid(), false );
		if( pCache )
		{
			pCache->BDestroyFromMsg( *m_pGCClient, msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSODestroyJob, "CGCSODestroyJob", k_ESOMsg_Destroy );

class CGCSOUpdateJob : public CGCClientJob
{
public:
	CGCSOUpdateJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOSingleObject> msg( pNetPacket );
		SOCDebug( "CGCSOUpdateJob(owner=%s, type=%d)\n", SOIDRender_t( msg.Body().owner_soid() ).String(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( msg.Body().owner_soid() );
		if ( pSOCache )
		{
			pSOCache->BUpdateFromMsg( *m_pGCClient, msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOUpdateJob, "CGCSOUpdateJob", k_ESOMsg_Update );

class CGCSOUpdateMultipleJob : public CGCClientJob
{
public:
	CGCSOUpdateMultipleJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOMultipleObjects> msg( pNetPacket );
		SOCDebug( "CGCSOUpdateJob(owner=%s)\n", SOIDRender_t( msg.Body().owner_soid() ).String() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( msg.Body().owner_soid() );
		if ( pSOCache )
		{
			for ( int i = 0; i < msg.Body().objects_added_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_added( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BCreateFromMsg( *m_pGCClient, objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			for ( int i = 0; i < msg.Body().objects_modified_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_modified( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BUpdateFromMsg( *m_pGCClient, objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			for ( int i = 0; i < msg.Body().objects_removed_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_removed( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BDestroyFromMsg( *m_pGCClient, objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOUpdateMultipleJob, "CGCSOUpdateMultipleJob", k_ESOMsg_UpdateMultiple );

class CGCSOCacheSubscribedJob : public CGCClientJob
{
public:
	CGCSOCacheSubscribedJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheSubscribed > msg ( pNetPacket );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( msg.Body().owner_soid(), true );

		Assert( pSOCache );
		if( pSOCache )
		{
			SOCDebug( "CGCSOCacheSubscribedJob(owner=%s) [in cache]\n", SOIDRender_t( msg.Body().owner_soid() ).String() );
			DbgVerify( pSOCache->BParseCacheSubscribedMsg( *m_pGCClient, msg.Body() ) );
		}
		else
		{
			SOCDebug( "CGCSOCacheSubscribedJob(owner=%s) [not in cache]\n", SOIDRender_t( msg.Body().owner_soid() ).String() );
		}

		m_pGCClient->Test_CacheSubscribed( pSOCache->GetOwner() );

		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheSubscribedJob, "CGCSOCacheSubscribedJob", k_ESOMsg_CacheSubscribed );

class CGCSOCacheUnsubscribedJob : public CGCClientJob
{
public:
	CGCSOCacheUnsubscribedJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheUnsubscribed > msg( pNetPacket );
		SOCDebug( "CGCSOCacheUnsubscribedJob(owner=%s)\n", SOIDRender_t( msg.Body().owner_soid() ).String() );
		m_pGCClient->NotifySOCacheUnsubscribed( msg.Body().owner_soid() );

		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheUnsubscribedJob, "CGCSOCacheUnsubscribedJob", k_ESOMsg_CacheUnsubscribed );

class CGCSOCacheSubscriptionCheck : public CGCClientJob
{
public:
	CGCSOCacheSubscriptionCheck( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheSubscriptionCheck > msg ( pNetPacket );
		m_pGCClient->ProcessCacheSubscriptionCheckMsg( msg.Body() );
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheSubscriptionCheck, "CGCSOCacheSubscriptionCheck", k_ESOMsg_CacheSubscriptionCheck );

} // namespace GCSDK
