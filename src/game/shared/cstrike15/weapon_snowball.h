#ifndef WEAPON_SNOWBALL_H
#define WEAPON_SNOWBALL_H

#include "weapon_basecsgrenade.h"

#if defined( CLIENT_DLL )
	#define CSnowball C_Snowball
#endif

//-----------------------------------------------------------------------------
// Snowball
//-----------------------------------------------------------------------------
class CSnowball : public CBaseCSGrenade
{
public:
	DECLARE_CLASS( CSnowball, CBaseCSGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CSnowball() {}

	virtual CSWeaponID GetCSWeaponID( void ) const { return WEAPON_SNOWBALL; }

#if defined( CLIENT_DLL )

#else // GAME_DLL
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, const CCSWeaponInfo &weaponInfo );
#endif // GAME_DLL

	CSnowball( const CSnowball & ) {}
};

#endif // WEAPON_SNOWBALL_H
