//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef CS_GC_CLIENT_H
#define CS_GC_CLIENT_H
#ifdef _WIN32
#pragma once
#endif

#if !defined( _X360 ) && !defined( NO_STEAM )
#include "steam/steam_api.h"
#endif

#include "gcsdk/gcclientsdk.h"
#include "cstrike15_gcmessages.pb.h"
#include "../clientsteamcontext.h"
#include "../gc_clientsystem.h"
#include "GameEventListener.h"
#include "econ_game_account_client.h"
#include "ns_address.h"

extern const char* g_szConfirmedReservationFilename;

class ConfirmedReservationData_t {
private:
	ns_address m_serverAddress;
	uint64 m_nReservationId;
	CSteamID m_nServerId;
	netadr_s m_directUdp;
	std::string m_map;

public:
	ConfirmedReservationData_t()
		: m_serverAddress()
		, m_nServerId()
		, m_directUdp()
	{
	}

	void SetPacketData(CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve& msg) const;
	void SetReservationData(const CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve& msg);

	void Clear() const;
	void Write() const;
	bool Load();
};

class CCSGCClientSystem : public CGCClientSystem, public GCSDK::ISharedObjectListener
{
	DECLARE_CLASS_GAMEROOT( CCSGCClientSystem, CGCClientSystem );
public:
	CCSGCClientSystem( void );
	~CCSGCClientSystem( void );

	// CAutoGameSystemPerFrame
	virtual bool Init() OVERRIDE;
	virtual void PostInit() OVERRIDE;
	virtual void LevelInitPreEntity() OVERRIDE;
	virtual void LevelShutdownPostEntity() OVERRIDE;
	virtual void Shutdown() OVERRIDE;
	virtual void Update( float frametime ) OVERRIDE;

	// CGCClientSystem
	virtual void PreInitGC() OVERRIDE;
	virtual void PostInitGC() OVERRIDE;

	// ISharedObjectListener
	virtual void SOCreated( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void PreSOUpdate( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SOUpdated( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void PostSOUpdate( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SODestroyed( const CSteamID & steamIDOwner, const GCSDK::CSharedObject *pObject, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* do nothing */ }
	virtual void SOCacheSubscribed( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { /* TODO */ }
	virtual void SOCacheUnsubscribed( const CSteamID & steamIDOwner, GCSDK::ESOCacheEvent eEvent ) OVERRIDE { m_pSOCache = NULL; }

	// this
	void GetTimePlayedConsecutively() const;
	void UpdateSteamAppDisableUpdate();

	virtual bool OnReceivedWelcomeMessage(CMsgClientWelcome& msgClientWelcome, CMsgCStrike15Welcome& msgCSWelcome);
	virtual bool OnReceivedMatchmakingWelcomeMessage(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msgGC2ClientHello);

private:
	GCSDK::CGCClientSharedObjectCache* m_pSOCache;
	int m_nTimePlayedConsecutively;
	int m_nLastTimePlayed;
	netadr_s m_ip;
};

CCSGCClientSystem* CSGCClientSystem();

#endif // CS_GC_CLIENT_H
