//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Extra functionality on top of CGCClientSharedObjectCache for GCClients
//
//=============================================================================

#include "stdafx.h"
#include <time.h>
#include "gcsdk/gcclient_sharedobjectcache.h"
#include "gcsdk_gcmessages.pb.h"
#include <typeinfo>

namespace GCSDK
{

#define SOCDebug(...) Msg( __VA_ARGS__ )
//#define SOCDebug(...) ((void)0)

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CGCClientSharedObjectTypeCache::CGCClientSharedObjectTypeCache( int nTypeID )
	: CSharedObjectTypeCache( nTypeID )
{

}


//----------------------------------------------------------------------------
// Purpose: Destructor
//----------------------------------------------------------------------------
CGCClientSharedObjectTypeCache::~CGCClientSharedObjectTypeCache()
{
}


//----------------------------------------------------------------------------
// Purpose: Parses a cache subscribed message.
//----------------------------------------------------------------------------
bool CGCClientSharedObjectTypeCache::BParseCacheSubscribedMsg( const CMsgSOCacheSubscribed_SubscribedType & msg, CUtlVector<CSharedObject*> &vecCreatedObjects, CUtlVector<CSharedObject*> &vecUpdatedObjects, CUtlVector<CSharedObject*> &vecObjectsToDestroy )
{
	CSharedObjectVec vecUntouchedObjects;
	for ( uint32 i = 0; i < GetCount(); i++ )
	{
		vecUntouchedObjects.AddToTail( GetObject( i ) );
	}

	for( uint16 usObject = 0; usObject < msg.object_data_size(); usObject++ )
	{
		bool bUpdatedExisting = false;
		CSharedObject *pObject = BCreateFromMsg( msg.object_data( usObject ).data(), msg.object_data( usObject ).size(), &bUpdatedExisting );
		if ( pObject == NULL)
		{
			Assert( pObject );
			return false;
		}

		// if an object was updated, remove it from the untouched list
		if ( bUpdatedExisting )
		{
			int index = vecUntouchedObjects.Find( pObject );
			if ( index != vecUntouchedObjects.InvalidIndex() )
			{
				vecUntouchedObjects[index] = NULL;
			}
			vecUpdatedObjects.AddToTail( pObject );
		}
		else
		{
			vecCreatedObjects.AddToTail( pObject );
		}
	}

	// all objects that weren't in the SubscribedMsg should be destroyed
	for ( int i = 0; i < vecUntouchedObjects.Count(); i++ )
	{
		if ( vecUntouchedObjects[i] == NULL )
			continue;

		CSharedObject *pObject = RemoveObject( *vecUntouchedObjects[i] );
		Assert( pObject );
		if( pObject )
			vecObjectsToDestroy.AddToTail( pObject );
	}

	return true;
}

void CGCClientSharedObjectTypeCache::RemoveAllObjects( CUtlVector<CSharedObject*> &vecObjects )
{

	// Go in reverse order to avoid O(n^2) shifting the items in the array
	for ( int i = GetCount() - 1; i >= 0; i-- )
	{
		CSharedObject *pObject = RemoveObjectByIndex( i );
		Assert( pObject );
		if ( pObject )
			vecObjects.AddToTail( pObject );
	}
}


//----------------------------------------------------------------------------
// Purpose: Processes a received create message for an object of this type on
//			the client/gameserver
//----------------------------------------------------------------------------
CSharedObject *CGCClientSharedObjectTypeCache::BCreateFromMsg( const void *pvData, uint32 unSize, bool *bUpdatedExisting )
{
	CUtlBuffer bufCreate( pvData, unSize, CUtlBuffer::READ_ONLY );
	CSharedObject *pNewObj = CSharedObject::Create( GetTypeID() );
	Assert( pNewObj );
	if( !pNewObj )
	{
		EmitError( SPEW_SHAREDOBJ, "Unable to create object of type %d\n", GetTypeID() );
		return NULL;
	}

	if( !pNewObj->BParseFromMessage( bufCreate ) )
	{
		delete pNewObj;
		return NULL;
	}

	// Existing object?
	CSharedObject *pObj = FindSharedObject( *pNewObj );
	if( pObj )
	{
		pObj->Copy( *pNewObj );
		delete pNewObj;
		if ( bUpdatedExisting )
		{
			*bUpdatedExisting = true;
		}
		return pObj;
	}

	// New object
	AddObject( pNewObj );
	if ( bUpdatedExisting )
	{
		*bUpdatedExisting = false;
	}
	return pNewObj;
}


//----------------------------------------------------------------------------
// Purpose: Processes a received destroy message for an object of this type on
//			the client/gameserver
//----------------------------------------------------------------------------
bool CGCClientSharedObjectTypeCache::BDestroyFromMsg( SOID_t owner, CGCClient &client, const void *pvData, uint32 unSize )
{
	CUtlBuffer bufDestroy( pvData, unSize, CUtlBuffer::READ_ONLY );
	CSharedObject *pIndexObj = CSharedObject::Create( GetTypeID() );
	if( !pIndexObj->BParseFromMessage( bufDestroy ) )
	{
		delete pIndexObj;
		return false;
	}

	CSharedObject *pObject = RemoveObject( *pIndexObj );
	if( pObject )
	{
		client.DispatchSODestroyed( owner, pObject, eSOCacheEvent_Incremental );
		delete pObject;
	}

	delete pIndexObj;
	return true;
}


//----------------------------------------------------------------------------
// Purpose: Processes a received destroy message for an object of this type on
//			the client/gameserver
//----------------------------------------------------------------------------
bool CGCClientSharedObjectTypeCache::BCreateOrUpdateFromMsg( SOID_t owner, CGCClient &client, const void *pvData, uint32 unSize )
{
	CUtlBuffer bufUpdate( pvData, unSize, CUtlBuffer::READ_ONLY );
	CSharedObject *pIndexObj = CSharedObject::Create( GetTypeID() );
	AssertMsg1( pIndexObj, "Unable to create index object of type %d", GetTypeID() );
	if( !pIndexObj )
		return false;
	if( !pIndexObj->BParseFromMessage( bufUpdate ) )
	{
		delete pIndexObj;
		return false;
	}

	CSharedObject *pObj = FindSharedObject( *pIndexObj );
	bool bRet = false;
	if( pObj )
	{
		bufUpdate.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );

		bRet = pObj->BUpdateFromNetwork( *pIndexObj );
		client.DispatchSOUpdated( owner, pObj, eSOCacheEvent_Incremental );
	}
	else
	{
		AddObject( pIndexObj );
		client.DispatchSOCreated( owner, pIndexObj, eSOCacheEvent_Incremental );
	}

	delete pIndexObj;
	return bRet;
}


//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CGCClientSharedObjectCache::CGCClientSharedObjectCache( SOID_t ID ) 
	: m_IDOwner( ID ),
	m_bInitialized( false ),
	m_bSubscribed( false )
{

}


//----------------------------------------------------------------------------
// Purpose: Destructor
//----------------------------------------------------------------------------
CGCClientSharedObjectCache::~CGCClientSharedObjectCache()
{
}


//----------------------------------------------------------------------------
// Purpose: Process an incoming create message on a client/gameserver.
//----------------------------------------------------------------------------
bool CGCClientSharedObjectCache::BParseCacheSubscribedMsg( CGCClient &owner, const CMsgSOCacheSubscribed & msg )
{

	// Assume all type caches will be untouched
	CUtlVector<int> vecUntouchedTypes;
	for ( int i = FirstTypeCacheIndex(); i != InvalidTypeCacheIndex(); i = NextTypeCacheIndex( i ) )
	{
		CSharedObjectTypeCache *pTypeCache = GetTypeCacheByIndex( i );
		if ( pTypeCache )
		{
			vecUntouchedTypes.AddToTail( pTypeCache->GetTypeID() );
		}
	}

	// List of objects created, updated, and removed
	CUtlVector<CSharedObject*> vecCreatedObjects;
	CUtlVector<CSharedObject*> vecUpdatedObjects;
	CUtlVector<CSharedObject*> vecObjectsToDestroy;

	bool bResult = true;

	// Scan types in message
	for( uint16 usObject = 0; usObject < msg.objects_size(); usObject++ )
	{
		const CMsgSOCacheSubscribed_SubscribedType & msgType = msg.objects( usObject );

		// Find or create the type
		CGCClientSharedObjectTypeCache *pTypeCache = CreateTypeCache( msgType.type_id() );
		if ( pTypeCache )
		{
			int index = vecUntouchedTypes.Find( pTypeCache->GetTypeID() );
			if ( index != vecUntouchedTypes.InvalidIndex() )
			{
				vecUntouchedTypes[index] = -1;
			}
		}
		Assert( pTypeCache );
		if( !pTypeCache || !pTypeCache->BParseCacheSubscribedMsg( msgType, vecCreatedObjects, vecUpdatedObjects, vecObjectsToDestroy ) )
			bResult = false;
	}

	// any type caches that weren't in the SubscribedMsg should be cleared
	for ( int i = FirstTypeCacheIndex(); i != InvalidTypeCacheIndex(); i = NextTypeCacheIndex( i ) )
	{
		CGCClientSharedObjectTypeCache *pTypeCache = GetTypeCacheByIndex( i );
		if ( vecUntouchedTypes.Find( pTypeCache->GetTypeID() ) != vecUntouchedTypes.InvalidIndex() )
		{
			pTypeCache->RemoveAllObjects( vecObjectsToDestroy );
		}
	}

	// Which event is happening?
	ESOCacheEvent eNotificationEvent = eSOCacheEvent_Subscribed;
	if ( m_bSubscribed )
		eNotificationEvent = eSOCacheEvent_Resubscribed;

	// Set version, assuming we didn't have any problems.  If we hit any problems,
	// we want to force a refresh
	if ( bResult )
		SetVersion( msg.version() );

	// Mark that the cache has been initialized by the server
	m_bInitialized = true;
	m_bSubscribed = true;

	//
	// Send notifications
	//

	// Initial cache subscribed
	owner.DispatchSOCacheSubscribed( GetOwner(), eNotificationEvent );

	// Deletions
	for ( int i = 0 ; i < vecObjectsToDestroy.Count() ; ++i )
	{
		owner.DispatchSODestroyed( GetOwner(), vecObjectsToDestroy[i], eNotificationEvent );
		delete vecObjectsToDestroy[i];
	}

	// Updates
	for ( int i = 0 ; i < vecUpdatedObjects.Count() ; ++i )
	{
		owner.DispatchSOUpdated( GetOwner(), vecUpdatedObjects[i], eNotificationEvent );
	}

	// Created
	for ( int i = 0 ; i < vecCreatedObjects.Count() ; ++i )
	{
		owner.DispatchSOUpdated( GetOwner(), vecCreatedObjects[i], eNotificationEvent );
	}

	// Return true if everything parsed OK, or false
	// if we had at least one failure
	return bResult;
}


//----------------------------------------------------------------------------
// Purpose: Process an incoming create message on a client/gameserver.
//----------------------------------------------------------------------------
bool CGCClientSharedObjectCache::BCreateFromMsg( CGCClient &owner, int nTypeID, const void *pvData, uint32 unSize )
{
	// We should be subscribed
	if ( !m_bInitialized || !m_bSubscribed )
	{
		// Note: We can go down and come back up without the GC knowing this.
		// So this can happen
		//Assert( m_bInitialized );
		//Assert( m_bSubscribed );
		//EmitWarning( SPEW_SHAREDOBJ, 1, "Received SOCache incremental update for cache we were not subscribed to (object type %d)\n", nTypeID );
	}

	// Locate / create the type cache
	CGCClientSharedObjectTypeCache *pTypeCache = CreateTypeCache( nTypeID );

	// Create the message or update existing
	bool bUpdatedExisting = false;
	CSharedObject *pObject = pTypeCache->BCreateFromMsg( pvData, unSize, &bUpdatedExisting );
	if ( pObject == NULL )
		return false;

	// Send notifications to listeners
	if ( bUpdatedExisting )
	{
		// This can happen --- see comment at the top of this function
		//Assert( !bUpdatedExisting ); // shouldn't the GC know what it's already sent us?  This is weird
		owner.DispatchSOUpdated( GetOwner(), pObject, eSOCacheEvent_Incremental );
	}
	else
	{
		owner.DispatchSOCreated( GetOwner(), pObject, eSOCacheEvent_Incremental );
	}

	return true;
}


//----------------------------------------------------------------------------
// Purpose: Processes an incoming destroy message on a client/gameserver.
//----------------------------------------------------------------------------
bool CGCClientSharedObjectCache::BDestroyFromMsg( CGCClient &owner, int nTypeID, const void *pvData, uint32 unSize )
{
	CGCClientSharedObjectTypeCache *pTypeCache = FindTypeCache( nTypeID );
	if( pTypeCache )
	{
		return pTypeCache->BDestroyFromMsg( GetOwner(), owner, pvData, unSize );
	}
	else
	{
		return false;
	}	
}

//----------------------------------------------------------------------------
// Purpose: Processes an incoming update message on a client/gameserver.
//----------------------------------------------------------------------------
bool CGCClientSharedObjectCache::BUpdateFromMsg( CGCClient &owner, int nTypeID, const void *pvData, uint32 unSize )
{
	CGCClientSharedObjectTypeCache *pTypeCache = FindTypeCache( nTypeID );
	if( pTypeCache )
	{
		return pTypeCache->BCreateOrUpdateFromMsg( GetOwner(), owner, pvData, unSize );
	}
	else
	{
		return false;
	}	
}

void CGCClientSharedObjectCache::NotifyCreated(ISharedObjectListener& context)
{
	context.SOCacheSubscribed( GetOwner(), eSOCacheEvent_ListenerAdded );

	for ( int i = FirstTypeCacheIndex(); i != InvalidTypeCacheIndex(); i = NextTypeCacheIndex( i ) )
	{
		CGCClientSharedObjectTypeCache *pTypeCache = GetTypeCacheByIndex( i );
		
		int nObjectsCount = pTypeCache->GetCount();
		for ( int nObj = nObjectsCount; nObj < nObjectsCount; nObj++ )
		{
			CSharedObject *pObj = pTypeCache->GetObject( nObj );
			context.SOCreated( GetOwner(), pObj, eSOCacheEvent_ListenerAdded );
		}
	}
}

}  // namespace GCSDK
