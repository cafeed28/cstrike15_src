#include "cbase.h"
#include "snowball_pile.h"

#if defined( CLIENT_DLL )
	#include "c_cs_player.h"
#else // GAME_DLL
	#include "cs_player.h"
#endif

#define SNOWBALL_PILE_MODEL "models/props_holidays/snowball/snowball_pile.mdl"

PRECACHE_REGISTER( ent_snowball_pile );

#if defined( CLIENT_DLL )

IMPLEMENT_CLIENTCLASS_DT( C_SnowballPile, DT_SnowballPile, CSnowballPile )
END_RECV_TABLE()

void C_SnowballPile::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( !m_hEffect )
	{
		m_hEffect = ParticleProp()->Create( "snowball_pile", PATTACH_ABSORIGIN_FOLLOW );
	}
}

#else // GAME_DLL

LINK_ENTITY_TO_CLASS( ent_snowball_pile, CSnowballPile );

IMPLEMENT_SERVERCLASS_ST( CSnowballPile, DT_SnowballPile )
END_SEND_TABLE()

BEGIN_DATADESC( CSnowballPile )
	DEFINE_USEFUNC( SnowPileUse ),
END_DATADESC()

void CSnowballPile::SnowPileUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
		return;
	
	CCSPlayer* pCSPlayer = ToCSPlayer( pActivator );
	if ( !pCSPlayer )
		return;

	if ( pCSPlayer->CanAcquire( WeaponIdFromString( "weapon_snowball" ), AcquireMethod::PickUp ) == AcquireResult::Allowed )
	{
		pCSPlayer->GiveNamedItem( "weapon_snowball" );
	}
}

void CSnowballPile::Spawn( void )
{
	Precache();
	
	SetModel( SNOWBALL_PILE_MODEL );
	SetUse( &CSnowballPile::SnowPileUse );
	
	SetSolid( SOLID_VPHYSICS );
	SetMoveType( MOVETYPE_NONE );
	SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );

	AddFlag( FL_OBJECT );
	
	SetCollisionBounds( Vector( -10, -10, 0 ), Vector( 10, 10, 32 ) );
	
	m_takedamage = DAMAGE_NO;
}

void CSnowballPile::Precache( void )
{
	PrecacheModel( SNOWBALL_PILE_MODEL );
	PrecacheParticleSystem( "snowball_pile" );

	BaseClass::Precache();
}

#endif // GAME_DLL
