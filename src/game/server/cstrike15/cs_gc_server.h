//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef CS_GC_SERVER_H
#define CS_GC_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#if !defined( _X360 ) && !defined( NO_STEAM )
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#endif

#include "gcsdk/gcclientsdk.h"
#include "playergroup.h"
#include "gc_clientsystem.h"
#include "cstrike15_gcmessages.pb.h"
#include "GameEventListener.h"
#include "rtime.h"

class CCSGCServerSystem : public CGCClientSystem, public GCSDK::ISharedObjectListener
{
	DECLARE_CLASS_GAMEROOT( CCSGCServerSystem, CGCClientSystem );

public:
	CCSGCServerSystem( void );
	~CCSGCServerSystem( void );

	// CAutoGameSystemPerFrame
	virtual bool Init() OVERRIDE;
	virtual void PostInit() OVERRIDE;
	virtual void LevelInitPreEntity() OVERRIDE;
	virtual void LevelShutdownPostEntity() OVERRIDE;
	virtual void Shutdown() OVERRIDE;
	virtual void PreClientUpdate() OVERRIDE;

	// CGCClientSystem
	virtual void PreInitGC() OVERRIDE;
	virtual void PostInitGC() OVERRIDE;

	// ISharedObjectListener
	virtual void SOCreated( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void PreSOUpdate( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SOUpdated( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void PostSOUpdate( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SODestroyed( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SOCacheSubscribed( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SOCacheUnsubscribed( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }

private:
	friend class CGCClientJobServerWelcome;

	char pad[20];
	CSteamID m_steamID;
	char pad2[16];
	int m_nUnk;
	char pad3[4];
	uint32 m_nSessionInstanceIndex;
};

CCSGCServerSystem *CSGCClientSystem();

#endif // CS_GC_SERVER_H
