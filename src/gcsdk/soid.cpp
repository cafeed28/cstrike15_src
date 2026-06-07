//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================


#include "stdafx.h"

#include "tier0/memdbgon.h"

namespace GCSDK
{

struct CSOIDNameInfo
{
	const char* szName;
	bool bDisplaySteamID;
};

static CUtlMap< uint32, CSOIDNameInfo > &GSOIDNameMap()
{
	static CUtlMap< uint32, CSOIDNameInfo > s_SOIDNameMap( DefLessFunc( uint32 ) );
	return s_SOIDNameMap;
}

SOIDRender_t::CAutoRegisterName g_RegisterLock_SteamID( k_SOID_Type_SteamID, "SteamID", true );
SOIDRender_t::CAutoRegisterName g_RegisterLock_Party( k_SOID_Type_PartyGroupID, "Party" );
SOIDRender_t::CAutoRegisterName g_RegisterLock_Lobby( k_SOID_Type_LobbyGroupID, "Lobby" );
SOIDRender_t::CAutoRegisterName g_RegisterLock_PartyInvite( k_SOID_Type_PartyInvite, "Invite" );
SOIDRender_t::CAutoRegisterName g_RegisterLock_CheatReport( k_SOID_Type_CheatReport, "Report" );

SOIDRender_t::CAutoRegisterName::CAutoRegisterName( uint16 nType, const char* pszDefaultString, bool bDisplaySteamID )
{
	if ( GSOIDNameMap().Find( nType ) == GSOIDNameMap().InvalidIndex() )
	{
		int nIndex = GSOIDNameMap().Insert( nType );
		GSOIDNameMap()[ nIndex ].szName = pszDefaultString;
		GSOIDNameMap()[ nIndex ].bDisplaySteamID = bDisplaySteamID;
	}
	else
	{
		AssertMsg( false, "Warning: Had multiple definitions registered for the SOID name of type %d: String %s", nType, pszDefaultString );
	}
}

const char *SOIDRender_t::GetName( uint32 nType )
{
	if ( GSOIDNameMap().Find( nType ) == GSOIDNameMap().InvalidIndex() )
		return NULL;
	return GSOIDNameMap()[ nType ].szName;
}

SOIDRender_t::SOIDRender_t( const SOID_t id )
{
	const char *szName = NULL;
	bool bDisplaySteamID = false;
	
	if ( GSOIDNameMap().Find( id.m_type ) != GSOIDNameMap().InvalidIndex() )
	{
		szName = GSOIDNameMap()[ id.m_type ].szName;
		bDisplaySteamID = GSOIDNameMap()[ id.m_type ].bDisplaySteamID;
	}

	if ( szName )
	{
		if ( bDisplaySteamID )
		{
			V_snprintf( m_buf, k_cBufLen, "<%s:%s>", szName, CSteamID( id.ID() ).Render() );
		}
		else
		{
			V_snprintf( m_buf, k_cBufLen, "<%s:%llu>", szName, id.ID() );
		}
	}
	else
	{
		V_snprintf( m_buf, k_cBufLen, "<%d:%llu>", id.Type(), id.ID() );
	}
}

SOID_t::SOID_t( const CMsgSOIDOwner &msgSOIDOwner )
: m_type( msgSOIDOwner.type() )
, m_id( msgSOIDOwner.id() )
, m_padding( 0 )
{
}

void SOID_t::ToMsgSOIDOwner( CMsgSOIDOwner *pMsgSOIDOwner ) const
{
	if ( pMsgSOIDOwner )
	{
		pMsgSOIDOwner->set_type( m_type );
		pMsgSOIDOwner->set_id( m_id );
	}
}

} // namespace GCSDK
