#ifndef SNOWBALL_PROJECTILE_H
#define SNOWBALL_PROJECTILE_H

#include "basecsgrenade_projectile.h"

#if defined( CLIENT_DLL )
#include "c_props.h"
#else // GAME_DLL
#include "props.h"
#endif

#if defined( CLIENT_DLL )

class C_SnowballProjectile : public C_BaseCSGrenadeProjectile//, public C_BreakableProp
{
public:
	DECLARE_CLASS( C_SnowballProjectile, C_BaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();

	virtual bool Simulate( void );

	virtual void OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect );
	virtual void OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect );

private:
	CUtlReference<CNewParticleEffect> m_snowballParticleEffect;
};

#else // GAME_DLL

struct SnowballWeaponProfile;

class CSnowballProjectile : public CBaseCSGrenadeProjectile//, public CBreakableProp
{
public:
	DECLARE_CLASS( CSnowballProjectile, CBaseCSGrenadeProjectile );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

// Overrides.
public:
	CSnowballProjectile();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Detonate( void );
	virtual void BounceTouch( CBaseEntity *other );
	
	void	InputSetTimer( inputdata_t &inputdata );

	virtual GrenadeType_t GetGrenadeType( void ) { return GRENADE_TYPE_SENSOR; }

// Grenade stuff.
	static CSnowballProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		const CCSWeaponInfo& weaponInfo );	

private:
	float m_flTimeToDetonate;

	// Count of players effected by the flash
	uint8 m_numOpponentsHit; // note: opponents are considered to be anybody not on the flasher's team.
	uint8 m_numTeammatesHit;
};

#endif // GAME_DLL

#endif // SNOWBALL_PROJECTILE_H
