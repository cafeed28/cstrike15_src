#include "cbase.h"
#include "snowball_projectile.h"
#include "weapon_csbase.h"
#include "particle_parse.h"

#if defined( CLIENT_DLL )
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
#endif

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

#if defined( CLIENT_DLL )

IMPLEMENT_CLIENTCLASS_DT( C_SnowballProjectile, DT_SnowballProjectile, CSnowballProjectile )
END_RECV_TABLE()

void C_SnowballProjectile::OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect )
{
	if ( FStrEq( pszParticleName, "weapon_snowball_trail" ) )
	{
		m_snowballParticleEffect = pNewParticleEffect;
	}
}

void C_SnowballProjectile::OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect )
{
	if ( m_snowballParticleEffect == pParticleEffect )
	{
		m_snowballParticleEffect = NULL;
	}
}

bool C_SnowballProjectile::Simulate( void )
{
	if ( !m_snowballParticleEffect.IsValid() )
	{
		DispatchParticleEffect( "weapon_snowball_trail", PATTACH_ABSORIGIN_FOLLOW, this );
	}
	else
	{
		m_snowballParticleEffect->SetSortOrigin( GetAbsOrigin() );
		m_snowballParticleEffect->SetNeedsBBoxUpdate( true );
	}

	BaseClass::Simulate();
	return true;
}

#else // GAME_DLL

#define GRENADE_MODEL "models/weapons/w_snowball.mdl"

LINK_ENTITY_TO_CLASS( snowball_projectile, CSnowballProjectile );
PRECACHE_REGISTER( snowball_projectile );

IMPLEMENT_SERVERCLASS_ST( CSnowballProjectile, DT_SnowballProjectile )
END_SEND_TABLE()

BEGIN_DATADESC( CSnowballProjectile )
END_DATADESC()

void RadiusSnowFlash(
	Vector vecSrc,
	CBaseEntity *pevInflictor,
	CBaseEntity *pevAttacker,
	float flDamage,
	int iClassIgnore,
	int bitsDamageType,
	uint8 *pOutNumOpponentsEffected = NULL,
	uint8 *pOutNumTeammatesEffected = NULL )
{	
	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( !pevAttacker )
		pevAttacker = pevInflictor;

	if ( pOutNumOpponentsEffected )
		*pOutNumOpponentsEffected = 0;

	if ( pOutNumTeammatesEffected )
		*pOutNumTeammatesEffected = 0;
	
	trace_t		tr;
	float		flAdjustedDamage;
	variant_t	var;
	Vector		vecEyePos;
	float		fadeTime, fadeHold;
	bool		bUnk;
	Vector		vForward;
	Vector		vecLOS;
	float		flDot;
	
	CBaseEntity		*pEntity = NULL;
	static float	flRadius = 62;
	float			falloff = flDamage / flRadius;
	// iterate on all entities in the vicinity.
	while ((pEntity = gEntList.FindEntityInSphere( pEntity, vecSrc, flRadius )) != NULL)
	{
		if( !pEntity->IsPlayer() )
			continue;

		CCSPlayer *player = static_cast< CCSPlayer * >( pEntity );

		vecEyePos = player->EyePosition();

		Ray_t ray;
		trace_t tr;
		CTraceFilterLOS traceFilter( pevInflictor, COLLISION_GROUP_NONE );
		unsigned int FLASH_MASK = MASK_OPAQUE_AND_NPCS | CONTENTS_DEBRIS;

		// According to comment in IsNoDrawBrush in cmodel.cpp, CONTENTS_OPAQUE is ONLY used for block light surfaces,
		// and we want flashbang traces to pass through those, since the block light surface is only used for blocking
		// lightmap light rays during map compilation.
		FLASH_MASK &= ~CONTENTS_OPAQUE;

		ray.Init( vecEyePos, vecSrc );
		enginetrace->TraceRay( ray, FLASH_MASK, &traceFilter, &tr );

		if ( tr.fraction == 1 )
		{
			if ( pOutNumOpponentsEffected && player->GetTeamNumber() != pevAttacker->GetTeamNumber() )
				(*pOutNumOpponentsEffected)++;
			if ( pOutNumTeammatesEffected && player->GetTeamNumber() == pevAttacker->GetTeamNumber() )
				(*pOutNumTeammatesEffected)++;

			// decrease damage for an ent that's farther from the grenade
			flAdjustedDamage = flDamage - ( vecSrc - player->EyePosition() ).Length() * falloff;
			if ( flAdjustedDamage > 0 )
			{
				// See if we were facing the flash
				AngleVectors( player->EyeAngles(), &vForward );

				vecLOS = ( vecSrc - vecEyePos );

				float flDistance = vecLOS.Length();

				// Normalize both vectors so the dotproduct is in the range -1.0 <= x <= 1.0 
				vecLOS.NormalizeInPlace();


				flDot = DotProduct (vecLOS, vForward);
	
				// if target is facing the bomb, the effect lasts longer
				if( flDot >= 0.6 )
				{
					// looking at the flashbang
					fadeTime = flAdjustedDamage;
					fadeHold = flAdjustedDamage * 0.1f;
					bUnk = flDistance < 36.0;
				}
				else if( flDot >= 0.3 )
				{
					// looking to the side
					fadeTime = flAdjustedDamage * 0.8f;
					fadeHold = flAdjustedDamage * 0.1f;
					bUnk = flDistance < 36.0;
				}
				else if( flDot < 0 )
				{
					// looking to the side
					fadeTime = flAdjustedDamage * 0.2f;
					fadeHold = flAdjustedDamage * 0.01f;
					bUnk = false;
				}
				else
				{
					// facing away
					fadeTime = flAdjustedDamage * 0.5f;
					fadeHold = flAdjustedDamage * 0.1f;
					bUnk = false;
				}
				
				player->SnowBlind( bUnk, fadeHold * falloff, fadeTime * falloff, 110 );
				player->EmitSound( "Snowball.HitPlayerFace" );

				if ( bUnk )
				{
					// blind players and bots
					IGameEvent * event = gameeventmanager->CreateEvent( "snowball_hit_player_face" );
					if ( event )
					{
						event->SetInt( "userid", player->GetUserID() );
						gameeventmanager->FireEvent( event );
					}
				}
			}	
		}
	}
}


CSnowballProjectile* CSnowballProjectile::Create( 
	const Vector &position, 
	const QAngle &angles, 
	const Vector &velocity, 
	const AngularImpulse &angVelocity, 
	CBaseCombatCharacter *pOwner,
	const CCSWeaponInfo& weaponInfo )
{
	CSnowballProjectile *pGrenade = ( CSnowballProjectile* )CBaseEntity::Create( "snowball_projectile", position, angles, pOwner );
	
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
	pGrenade->SetThrower( pOwner );

	pGrenade->ChangeTeam( pOwner->GetTeamNumber() );
	pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );

	pGrenade->m_pWeaponInfo = &weaponInfo;

	pGrenade->SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	pGrenade->SetCollisionBounds( Vector( -3, -3, -3 ), Vector( 3, 3, 3 ) );
	return pGrenade;
}

void CSnowballProjectile::Spawn( void )
{
	SetModel( GRENADE_MODEL );

	SetTouch( &CSnowballProjectile::BounceTouch );

	SetThink( &CBaseCSGrenadeProjectile::DangerSoundThink );
	SetNextThink( gpGlobals->curtime );

	SetGravity( BaseClass::GetGrenadeGravity() );
	SetFriction( BaseClass::GetGrenadeFriction() );
	SetElasticity( BaseClass::GetGrenadeElasticity() );

	m_pWeaponInfo = GetWeaponInfo( WEAPON_SNOWBALL );

	BaseClass::Spawn();

	SetBodygroupPreset( "thrown" );
}

CSnowballProjectile::CSnowballProjectile()
{
	// m_flSmth1_NetVar = 1.0; // 0x3F800000
	// m_flSmth2 = 10.0; // 0x41200000
	m_numOpponentsHit = m_numTeammatesHit = 0;
}

void CSnowballProjectile::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );

	PrecacheScriptSound( "Player.SnowballHit" );
	PrecacheScriptSound( "Snowball.HitPlayerFace" );
	PrecacheScriptSound( "Snowball.Bounce" );

	PrecacheParticleSystem( "weapon_snowball_impact" );

	BaseClass::Precache();
}
ConVar sv_snowball_strength( "sv_snowball_strength", "12.0", FCVAR_REPLICATED, "Snowball strength", true, 2.0, true, 64.0 );

void CSnowballProjectile::Detonate( void )
{
	RadiusSnowFlash ( GetAbsOrigin(), this, GetThrower(), sv_snowball_strength.GetInt(), CLASS_NONE, DMG_BLAST, &m_numOpponentsHit, &m_numTeammatesHit );
	EmitSound("Player.SnowballHit");
	
	Vector vecStart = GetAbsOrigin();

	Vector vecDirection = GetAbsVelocity();
	VectorNormalize( vecDirection );
	Vector vecEnd = vecStart + vecDirection * 16.0f;

	trace_t tr;
	UTIL_TraceLine( vecStart, vecEnd, 0x600400B, this, NULL, &tr );

	QAngle ang;
	VectorAngles( tr.DidHit() ? tr.plane.normal : -vecDirection, ang );
	DispatchParticleEffect( "weapon_snowball_impact", GetAbsOrigin(), ang );

	if ( tr.DidHitWorld() )
	{
		Vector forward, right, up;
		AngleVectors( ang, &forward, &right, &up );
		VectorAngles( up, ang );
		DispatchParticleEffect( "weapon_snowball_impact_splat", GetAbsOrigin(), ang );
	}

	UTIL_Remove( this );
}

void CSnowballProjectile::BounceTouch( CBaseEntity *other )
{
	if ( other->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) )
		return;

	// don't hit the guy that launched this grenade
	if ( other == GetThrower() )
		return;

	// only do damage if we're moving fairly fast
	if ( ( other->m_takedamage != DAMAGE_NO ) && ( m_flNextAttack < gpGlobals->curtime && GetAbsVelocity().Length() > 100 ) )
	{
		if ( GetThrower() )
		{
#if !defined( CLIENT_DLL )
			trace_t tr;
			tr = CBaseEntity::GetTouchTrace( );
			ClearMultiDamage( );
			Vector forward;
			AngleVectors( GetLocalAngles(), &forward, NULL, NULL );
			CTakeDamageInfo info( this, GetThrower(), 1, DMG_CLUB );
			CalculateMeleeDamageForce( &info, GetAbsVelocity(), GetAbsOrigin() );
			other->DispatchTraceAttack( info, forward, &tr ); 
			ApplyMultiDamage();
#endif
		}
		m_flNextAttack = gpGlobals->curtime + 1.0; // debounce
	}

	if ( FClassnameIs( other, "func_breakable" ) )
	{
		return;
	}

	if ( FClassnameIs( other, "func_breakable_surf" ) )
	{
		return;
	}

	// don't detonate on ladders
	if ( FClassnameIs( other, "func_ladder" ) )
	{
		return;
	}

	Detonate();
}

#endif // GAME_DLL
