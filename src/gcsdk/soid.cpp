#include "stdafx.h"

#include "soid.h"
#include "gcsdk_gcmessages.pb.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace GCSDK
{

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

}