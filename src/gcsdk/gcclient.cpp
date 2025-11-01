//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Holds the CGCClient class
//
//=============================================================================

#include "stdafx.h"
#include "gcclient.h"
#include "gcsdk_gcmessages.pb.h"
#include "steam/isteamgamecoordinator.h"
#include "steam/isteamgameserver.h"
#include "steam/isteamuser.h"

namespace GCSDK
{

//#define SOCDebug(...) Msg( __VA_ARGS__ )
#define SOCDebug(...) ((void)0)

//------------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------------
CGCClient::CGCClient( ISteamGameCoordinator *pSteamGameCoordinator, bool bGameserver )
: m_pSteamGameCoordinator( NULL ),
	m_memMsg( 0, 1024 ),
#ifndef STEAM
	m_callbackGCMessageAvailable( NULL, NULL ),
#endif
	m_mapSOCache( DefLessFunc(CSteamID) ),
	m_unVersion( 0 ),
	m_nConnectionStatus( GCConnectionStatus_NO_STEAM ),
	m_bGameserver( bGameserver ),
	m_bSimulateGCConnectionFailure( false ),
	m_nSessionNeed( 0 ),
	m_nLastSessionNeed( -1 ),
	m_bWantSession( true ),
	m_nLauncherType( 0 ),
	m_usSteamdatagramPort( 0 ),
	m_nLogonQueuePosition( -1 ),
	m_nLogonQueueSize( -1 )
{
#ifndef STEAM
	if( bGameserver )
	{
		m_callbackGCMessageAvailable.SetGameserverFlag();
	}
#endif
	if( pSteamGameCoordinator )
	{
		//DbgVerify( BInit( pSteamGameCoordinator ) );
	}

	if ( CommandLine()->FindParm( "-perfectworld" ) )
	{
		m_nLauncherType = GCClientLauncherType_PERFECTWORLD;
	}

	m_timeLastSendHello.SetLTime( 0 );
	m_timeReceivedConnectionStatus.SetLTime( 0 );
	m_timeLoggedOn.SetLTime( 0 );
	m_timeLogonQueueEstimatedTimeExitQueue.SetLTime( 0 );
	m_timeLogonQueueApproxTimeEnteredQueue.SetLTime( 0 );
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
	return bWorkRemaining;
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
	if( m_pSteamGameCoordinator )
		return m_pSteamGameCoordinator->SendMessage( unMsgType, pubData, cubData ) == k_EGCResultOK;
	else
		return false;
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
		if( eResult == k_EGCResultOK )
		{
			if( unMsgType & k_EMsgProtoBufFlag )
			{
				CNetPacket *pGCPacket = CNetPacketPool::AllocNetPacket();
				pGCPacket->Init( cubData, pPacketFromGC );
				CIMsgNetPacketAutoRelease pMsgNetPacket( pGCPacket );

				// Safety check against malformed packet
				if ( pMsgNetPacket.Get() != NULL )
				{
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
					DispatchPacket( pMsgNetPacket.Get() );
				}

				// release the packet
				pGCPacket->Release();
			}
		}
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
		m_nLogonQueuePosition = m_nLogonQueueSize = -1;
		m_timeLogonQueueEstimatedTimeExitQueue.SetLTime( 0 );
		m_timeLogonQueueApproxTimeEnteredQueue.SetLTime( 0 );

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
	else if ( eMsg == k_EMsgGCServerWelcome && m_bGameserver )
	{
		EmitInfo( SPEW_GC, 1, 1, "Received server welcome from GC.\n" );
		
		m_nConnectionStatus = GCConnectionStatus_HAVE_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
		m_nLogonQueuePosition = m_nLogonQueueSize = -1;
		m_timeLogonQueueEstimatedTimeExitQueue.SetLTime( 0 );
		m_timeLogonQueueApproxTimeEnteredQueue.SetLTime( 0 );

		CProtoBufMsg< CMsgClientWelcome > msg( pMsgNetPacket );
		ProcessWelcomeMsg( msg.Body() );
	}
	else if ( eMsg == k_EMsgGCClientWelcome )
	{
		EmitInfo( SPEW_GC, 1, 1, "Received client welcome from GC.\n" );
		
		m_nConnectionStatus = GCConnectionStatus_HAVE_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
		m_nLogonQueuePosition = m_nLogonQueueSize = -1;
		m_timeLogonQueueEstimatedTimeExitQueue.SetLTime( 0 );
		m_timeLogonQueueApproxTimeEnteredQueue.SetLTime( 0 );

		CProtoBufMsg< CMsgClientWelcome > msg( pMsgNetPacket );
		ProcessWelcomeMsg( msg.Body() );
	}

	// dispatch the packet
	GetJobMgr().BRouteMsgToJob( this, pMsgNetPacket, JobMsgInfo_t( pMsgNetPacket->GetEMsg(), pMsgNetPacket->GetSourceJobID(), pMsgNetPacket->GetTargetJobID(), k_EServerTypeGCClient ) );

	// keep track of how much we've sent/received this message
	g_theMessageList.TallySendMessage( pMsgNetPacket->GetEMsg(), pMsgNetPacket->CubData() );
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
;
#ifndef STEAM
	m_callbackGCMessageAvailable.Register( this, &CGCClient::OnGCMessageAvailable );
	//m_CallbackSteamServersDisconnected.Register( this, &CGCClient::OnSteamServersDisconnected );
	//m_CallbackSteamServerConnectFailure.Register( this, &CGCClient::OnSteamServerConnectFailure );
	//m_CallbackSteamServersConnected.Register( this, &CGCClient::OnSteamServersConnected );
#endif

	bool bLoggedOn = false;
	if (m_pSteamUser)
	{
		bLoggedOn = m_pSteamUser->BLoggedOn();
	}
	else if (m_pSteamGameserver)
	{
		bLoggedOn = m_pSteamGameserver->BLoggedOn();
	}

	if (bLoggedOn)
	{
		m_timeLoggedOn.SetToJobTime();
		m_nConnectionStatus = GCConnectionStatus_NO_SESSION;
	}
	else
	{
		m_timeLoggedOn.SetLTime(0);
		m_nConnectionStatus = GCConnectionStatus_NO_STEAM;
	}

	if (m_nConnectionStatus != GCConnectionStatus_NO_STEAM)
	{
		m_nLogonQueuePosition = -1;
		m_nLogonQueueSize = -1;
		m_timeLogonQueueEstimatedTimeExitQueue.SetLTime(0);
		m_timeLogonQueueApproxTimeEnteredQueue.SetLTime(0);
	}

	OnGCMessageAvailable( NULL );

	int nWantSessionTimeout = -1;
	int nTimeout = 2;

	if (m_nConnectionStatus == GCConnectionStatus_NO_SESSION || m_nConnectionStatus == GCConnectionStatus_NO_STEAM)
	{
		return true;
	}

	if (m_bWantSession)
	{
		if (m_nConnectionStatus == GCConnectionStatus_NO_SESSION_IN_LOGON_QUEUE)
		{
			nWantSessionTimeout = 90;
		}
		else 
		{
			nWantSessionTimeout = 10;
		}
	}

	if (m_nLastSessionNeed == m_nSessionNeed)
	{
		if (nWantSessionTimeout <= 0)
			return true;
		
		nTimeout = nWantSessionTimeout;
	}

	if (m_timeLastSendHello.LTime() == 0)
	{
		SendHello();
		return true;
	}

	if (m_timeLastSendHello > m_timeReceivedConnectionStatus)
		m_timeReceivedConnectionStatus = m_timeLastSendHello;

	if (CJobTime::LJobTimeCur() > m_timeReceivedConnectionStatus.LTime() + nTimeout * k_nMillion)
		SendHello();
	
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
				pSOCache->NotifyUnsubscribe();
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
CGCClientSharedObjectCache *CGCClient::FindSOCache( const CSteamID & steamID, bool bCreateIfMissing )
{
	CUtlMap< CSteamID, CGCClientSharedObjectCache * >::IndexType_t nCache = m_mapSOCache.Find( steamID );
	if( m_mapSOCache.IsValidIndex( nCache ) )
		return m_mapSOCache[nCache];
	else
	{
		if( bCreateIfMissing )
		{
			Assert( steamID.IsValid() );
			if ( !steamID.IsValid() )
				return NULL;

			CGCClientSharedObjectCache *pCache = new CGCClientSharedObjectCache( steamID );
			m_mapSOCache.Insert( steamID, pCache );
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
void CGCClient::AddSOCacheListener( const CSteamID &ownerID, ISharedObjectListener *pListener )
{
	Assert( ownerID.IsValid() );
	CGCClientSharedObjectCache *pCache = FindSOCache( ownerID, true );
	Assert( pCache );
	pCache->AddListener( pListener );
}

//------------------------------------------------------------------------------
// Purpose: Remove listener from the SO cache, if he is listening
//------------------------------------------------------------------------------
bool CGCClient::RemoveSOCacheListener( const CSteamID &ownerID, ISharedObjectListener *pListener )
{
	Assert ( this != NULL );		// Damn people - check your pointers before calling!
	Assert( ownerID.IsValid() );
	CGCClientSharedObjectCache *pCache = FindSOCache( ownerID, false );
	if ( pCache == NULL )
		return false; // cache doesn't exist, so we could't have ben listening
	return pCache->RemoveListener( pListener );
}

//------------------------------------------------------------------------------
// Purpose: Notify that the given SO cache has been unsubscribed
//------------------------------------------------------------------------------
void CGCClient::NotifySOCacheUnsubscribed( const CSteamID & ownerID )
{

	CUtlMap< CSteamID, CGCClientSharedObjectCache * >::IndexType_t nCache = m_mapSOCache.Find( ownerID );
	if( m_mapSOCache.IsValidIndex( nCache ) )
	{

		CGCClientSharedObjectCache *pSOCache = m_mapSOCache[nCache];

		// Ignore requests to remove caches that were never subscribed
		if ( pSOCache->BIsSubscribed() )
		{
			SOCDebug( "NotifySOCacheUnsubscribed(%s) [in cache, subscribed]\n", ownerID.Render()  );
			pSOCache->NotifyUnsubscribe();
		}
		else
		{
			SOCDebug( "NotifySOCacheUnsubscribed(%s) [in cache, not subscribed]\n", ownerID.Render()  );
		}
	}
	else
	{
		SOCDebug( "NotifySOCacheUnsubscribed(%s) [not in cache]\n", ownerID.Render()  );
	}
}

//------------------------------------------------------------------------------
// Purpose: 
//------------------------------------------------------------------------------
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
		CProtoBufMsg< CMsgClientHello > msg( m_nLauncherType == GCClientLauncherType_PERFECTWORLD ? k_EMsgGCClientHelloPW : k_EMsgGCClientHello );
		msg.Body().set_version( m_unVersion );
		msg.Body().set_client_session_need( m_nSessionNeed );
		msg.Body().set_client_launcher( m_nLauncherType );
		
		FOR_EACH_MAP( m_mapSOCache, idx )
		{
			CMsgSOCacheHaveVersion* pMsg = msg.Body().mutable_socache_have_versions()->Add();

			SOID_t soid( m_mapSOCache[ idx ]->GetOwner() );
			soid.ToMsgSOIDOwner( pMsg->mutable_soid() );

			pMsg->set_version( m_mapSOCache[ idx ]->GetVersion() );
		}

		bSuccess = BSendMessage( msg );
	}
	else
	{
		CProtoBufMsg< CMsgServerHello > msg( k_EMsgGCServerHello );
		msg.Body().set_version( m_unVersion );
		msg.Body().set_steamdatagram_port( m_usSteamdatagramPort );
		msg.Body().set_client_launcher( m_nLauncherType );
		
		FOR_EACH_MAP( m_mapSOCache, idx )
		{
			CMsgSOCacheHaveVersion* pMsg = msg.Body().mutable_socache_have_versions()->Add();

			SOID_t soid( m_mapSOCache[ idx ]->GetOwner() );
			soid.ToMsgSOIDOwner( pMsg->mutable_soid() );

			pMsg->set_version( m_mapSOCache[ idx ]->GetVersion() );
		}

		bSuccess = BSendMessage( msg );
	}

	if ( !bSuccess )
	{
		EmitInfo( SPEW_GC, 1, 1, "Failed to send Hello message to the GC.\n" );
	}
}

void CGCClient::ProcessWelcomeMsg( const CMsgClientWelcome & msg )
{
	Msg( "ProcessWelcomeMsg: subbed caches size: uptodated = %d, outdated = %d\n",
		msg.uptodate_subscribed_caches_size(), msg.outofdate_subscribed_caches_size()
	);
}

void CGCClient::SetSimulateGCConnectionFailure( bool bForcedFailure )
{
	m_bSimulateGCConnectionFailure = bForcedFailure;
	if ( bForcedFailure )
	{
		EmitInfo( SPEW_GC, 1, 1, "Simulated GC communications failure is active.\n" );
		if ( m_nConnectionStatus != GCConnectionStatus_NO_STEAM )
			m_nConnectionStatus = GCConnectionStatus_NO_SESSION;
		m_timeReceivedConnectionStatus.SetToJobTime();
	}
	else
	{
		EmitInfo( SPEW_GC, 1, 1, "Simulated GC communications failure is not active.\n" );
	}
}

//------------------------------------------------------------------------------
// Purpose: Dump everything about everyone
//------------------------------------------------------------------------------
void CGCClient::Dump()
{
	FOR_EACH_MAP( m_mapSOCache, idx )
	{
		m_mapSOCache[ idx ]->Dump();
	}
}

//------------------------------------------------------------------------------
// Purpose: Finds the shared object for this steam ID and key object
//------------------------------------------------------------------------------
CSharedObject *CGCClient::FindSharedObject( const CSteamID & ownerID, const CSharedObject & soIndex ) 
{ 
	CGCClientSharedObjectCache *pCache = FindSOCache( ownerID, false );
	if( pCache )
		return pCache->FindSharedObject( soIndex ); 
	else
		return NULL;
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
		SOCDebug( "CGCSOCreateJob(owner=%s, type=%d)\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ) );
		if ( pSOCache )
		{
			pSOCache->BCreateFromMsg( msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCreateJob, "CGCSOCreateJob", k_ESOMsg_Create, GCSDK::k_EServerTypeGCClient );

class CGCSODestroyJob : public CGCClientJob
{
public:
	CGCSODestroyJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOSingleObject> msg( pNetPacket );
		SOCDebug( "CGCSODestroyJob(owner=%s, type=%d)\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ), false );
		if( pCache )
		{
			pCache->BDestroyFromMsg( msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSODestroyJob, "CGCSODestroyJob", k_ESOMsg_Destroy, GCSDK::k_EServerTypeGCClient );

class CGCSOUpdateJob : public CGCClientJob
{
public:
	CGCSOUpdateJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOSingleObject> msg( pNetPacket );
		SOCDebug( "CGCSOUpdateJob(owner=%s, type=%d)\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render(), msg.Body().type_id() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ) );
		if ( pSOCache )
		{
			pSOCache->BUpdateFromMsg( msg.Body().type_id(), msg.Body().object_data().data(), msg.Body().object_data().size() );
			Assert( msg.Body().has_version() );
			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOUpdateJob, "CGCSOUpdateJob", k_ESOMsg_Update, GCSDK::k_EServerTypeGCClient );

class CGCSOUpdateMultipleJob : public CGCClientJob
{
public:
	CGCSOUpdateMultipleJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg<CMsgSOMultipleObjects> msg( pNetPacket );
		SOCDebug( "CGCSOUpdateJob(owner=%s)\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ) );
		if ( pSOCache )
		{
			pSOCache->m_context.PreSOUpdate( eSOCacheEvent_Incremental );

			for ( int i = 0; i < msg.Body().objects_added_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_added( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BCreateFromMsg( objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			for ( int i = 0; i < msg.Body().objects_modified_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_modified( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BUpdateFromMsg( objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			for ( int i = 0; i < msg.Body().objects_removed_size(); ++i )
			{
				const CMsgSOMultipleObjects_SingleObject &objMessage = msg.Body().objects_removed( i );
				SOCDebug( "     type %d\n", objMessage.type_id() );
				pSOCache->BDestroyFromMsg( objMessage.type_id(), objMessage.object_data().data(), objMessage.object_data().size() );
			}

			pSOCache->m_context.PostSOUpdate( eSOCacheEvent_Incremental );
			pSOCache->SetVersion( msg.Body().version() );
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOUpdateMultipleJob, "CGCSOUpdateMultipleJob", k_ESOMsg_UpdateMultiple, GCSDK::k_EServerTypeGCClient );

class CGCSOCacheSubscribedJob : public CGCClientJob
{
public:
	CGCSOCacheSubscribedJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheSubscribed > msg ( pNetPacket );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ), true );

		Assert( pSOCache );
		if( pSOCache )
		{
			SOCDebug( "CGCSOCacheSubscribedJob(owner=%s) [in cache]\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );
			DbgVerify( pSOCache->BParseCacheSubscribedMsg( msg.Body() ) );
		}
		else
		{
			SOCDebug( "CGCSOCacheSubscribedJob(owner=%s) [not in cache]\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );
		}

		m_pGCClient->Test_CacheSubscribed( pSOCache->GetOwner() );

		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheSubscribedJob, "CGCSOCacheSubscribedJob", k_ESOMsg_CacheSubscribed, GCSDK::k_EServerTypeGCClient );

class CGCSOCacheUnsubscribedJob : public CGCClientJob
{
public:
	CGCSOCacheUnsubscribedJob( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheUnsubscribed > msg( pNetPacket );
		SOCDebug( "CGCSOCacheUnsubscribedJob(owner=%s)\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );
		m_pGCClient->NotifySOCacheUnsubscribed( GetSteamIDFromSOID( msg.Body().owner_soid() ) );

		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheUnsubscribedJob, "CGCSOCacheUnsubscribedJob", k_ESOMsg_CacheUnsubscribed, GCSDK::k_EServerTypeGCClient );

class CGCSOCacheSubscriptionCheck : public CGCClientJob
{
public:
	CGCSOCacheSubscriptionCheck( CGCClient *pClient ) : CGCClientJob( pClient ) {}

	virtual bool BYieldingRunGCJob( GCSDK::IMsgNetPacket *pNetPacket )
	{
		CProtoBufMsg< CMsgSOCacheSubscriptionCheck > msg ( pNetPacket );
		CGCClientSharedObjectCache *pSOCache = m_pGCClient->FindSOCache( GetSteamIDFromSOID( msg.Body().owner_soid() ), false );

		// if we do not have the cache or it is out-of-date, request a refresh
		if ( pSOCache == NULL || !pSOCache->BIsInitialized() || pSOCache->GetVersion() != msg.Body().version() )
		{
			SOCDebug( "CGCSOCacheSubscriptionCheck(owner=%s) -- need refresh\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );
			CProtoBufMsg< CMsgSOCacheSubscriptionRefresh > msg_response( k_ESOMsg_CacheSubscriptionRefresh );
			CMsgSOIDOwner owner_soid( msg.Body().owner_soid() );
			msg_response.Body().set_allocated_owner_soid( &owner_soid );
			m_pGCClient->BSendMessage( msg_response );
		}
		else
		{
			SOCDebug( "CGCSOCacheSubscriptionCheck(owner=%s) -- up-to-date, no refresh needed\n", CSteamID( GetSteamIDFromSOID( msg.Body().owner_soid() ) ).Render() );

			// This is one method by which the GC notifies us that we are subscribed.
			if ( !pSOCache->BIsSubscribed() )
			{
				pSOCache->NotifyResubscribedUpToDate();
				Assert( pSOCache->BIsSubscribed() );
			}
		}
		return true;
	}

};

GC_REG_JOB( CGCClient, CGCSOCacheSubscriptionCheck, "CGCSOCacheSubscriptionCheck", k_ESOMsg_CacheSubscriptionCheck, GCSDK::k_EServerTypeGCClient );

} // namespace GCSDK
