//========= Copyright Valve Corporation, All rights reserved. ============//
//
//
//
//=============================================================================
#ifndef GC_CLIENTSYSTEM_H
#define GC_CLIENTSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include <gcsdk/gcclientsdk.h>
#include "igamesystem.h"
#include "networkvar.h"
#include "rtime.h"
#include "gcsdk_gcmessages.pb.h"

#ifdef CLIENT_DLL
	#include "clientsteamcontext.h"
#endif

//=============================================================================
//
//	Client GC System.
//
//=============================================================================
class CGCClientSystem : public CAutoGameSystemPerFrame
{
	DECLARE_CLASS_GAMEROOT( CGCClientSystem, CAutoGameSystem );

public:

	// Constructor/Destructor.
	CGCClientSystem();
	~CGCClientSystem();
	
	void Activate() { SetupGC(); }

	// Init/Shutdown.
	virtual void PostInit() OVERRIDE;
	virtual void LevelInitPreEntity() OVERRIDE;
	virtual void LevelShutdownPostEntity() OVERRIDE;
	virtual void Shutdown() OVERRIDE;

	// Updates.  Gameservers do this at a slightly different place than clients
	#ifdef CLIENT_DLL
		virtual void Update( float frametime ) OVERRIDE;
	#else
		virtual void PreClientUpdate() OVERRIDE;
	#endif

	// Connection status
	bool BConnectedtoGC() const { return m_GCClient.m_nConnectionStatus == GCConnectionStatus_HAVE_SESSION; }
	bool HasReceivedGCHello() const { return m_bHasRecievedGCHello; }

	// GC Messages
	bool BSendMessage( uint32 unMsgType, const uint8 *pubData, uint32 cubData );
	bool BSendMessage( const GCSDK::CGCMsgBase& msg );
	bool BSendMessage( const GCSDK::CProtoBufMsgBase& msg );

	// GC SOCache
	GCSDK::CGCClientSharedObjectCache *GetSOCache( const GCSDK::SOID_t &id );
	GCSDK::CGCClientSharedObjectCache *FindOrAddSOCache( const GCSDK::SOID_t &id );

	// GC Client
	GCSDK::CGCClient *GetGCClient();

	RTime32 GCTimeFromLocalTime( RTime32 timeLocal );
	RTime32 GCTimeToLocalTime( RTime32 timeGC );

	// Steam
	#ifndef CLIENT_DLL
		void GameServerActivate();
	#endif

	virtual void ProcessWelcomeMessage( const CMsgClientWelcome &msg );

protected:

	void SetupGC();
	virtual void InitGC();
	virtual void PreInitGC() {}
	virtual void PostInitGC() {}

private:

	#ifdef CLIENT_DLL
		void SteamLoggedOnCallback( const SteamLoggedOnChange_t &loggedOnState );
	#else
		STEAM_GAMESERVER_CALLBACK( CGCClientSystem, OnLogonSuccess, SteamServersConnected_t, m_CallbackLogonSuccess );
	#endif

	bool m_bHasRecievedGCHello;

	RTime32 m_timeGCWelcomeTimestamp_RelativeToCRTime;
	RTime32 m_timeGCWelcomeTimestamp;
	double m_timeLastRecvWelcome;
	
	int m_nCurrency;
	CMsgClientWelcome_Location m_location;
	
	bool m_bInittedGC;
	GCSDK::CGCClient m_GCClient;

	friend class CGCClientSystemJob;
};


void SetGCClientSystem( CGCClientSystem* pGCClientSystem );
CGCClientSystem *GCClientSystem();

#endif // GC_CLIENTSYSTEM_H

