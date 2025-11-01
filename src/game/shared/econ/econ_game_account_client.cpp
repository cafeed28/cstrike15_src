//========= Copyright (c), Valve Corporation, All rights reserved. ============//
//
// Purpose: Code for the CEconGameAccountClient object
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "econ_game_account_client.h"
#if defined( CLIENT_DLL )
#include "gc_clientsystem.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


uint32 CEconGameAccountClient::ComputeXpBonusFlagsNow() const
{
	uint32 bonus_xp_usedflags = Obj().bonus_xp_usedflags();
	if ( GCClientSystem()->GCTimeFromLocalTime( CRTime::RTime32TimeCur() ) > Obj().bonus_xp_timestamp_refresh() )
		bonus_xp_usedflags &= 0x1000003C;
	return bonus_xp_usedflags;
}
