#ifndef SNOWBALL_PILE
#define SNOWBALL_PILE

#include "cbase.h"

#if defined( CLIENT_DLL )

class C_SnowballPile : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_SnowballPile, C_BaseAnimating );
	DECLARE_NETWORKCLASS();

	virtual void OnDataChanged( DataUpdateType_t updateType ) OVERRIDE;

private:
	HPARTICLEFFECT m_hEffect;
};

#else // GAME_DLL

class CSnowballPile : public CBaseAnimating
{
public:
	DECLARE_CLASS( CSnowballPile, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	DECLARE_DATADESC();
	
	virtual void Spawn( void ) OVERRIDE;
	virtual void Precache( void ) OVERRIDE;
	virtual int ObjectCaps(void ) OVERRIDE { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; };

	void SnowPileUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

#endif // GAME_DLL

#endif // SNOWBALL_PILE
