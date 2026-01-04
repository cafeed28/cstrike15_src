#include "cbase.h"
#include "weapon_snowball.h"

#if !defined( CLIENT_DLL )
	#include "snowball_projectile.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( Snowball, DT_Snowball )

BEGIN_NETWORK_TABLE( CSnowball, DT_Snowball )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CSnowball )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS_ALIASED( weapon_snowball, Snowball );
PRECACHE_REGISTER( weapon_snowball );

#if !defined( CLIENT_DLL )

	BEGIN_DATADESC( CSnowball )
	END_DATADESC()

	void CSnowball::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, const CCSWeaponInfo& weaponInfo )
	{
		CSnowballProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, weaponInfo );
	}

#endif
