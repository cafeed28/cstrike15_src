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
static CThreadMutex g_mutexTempTextBuffer;
static CUtlVector< char * > g_vecTempTextBuffer;

static bool g_bTextBuffersFreed;


char* GetPchTempTextBuffer()
{
	static CTHREADLOCALINTEGER( size_t ) s_irgchBuffer;
	static CTHREADLOCAL( char* ) s_rgtlsBuffers[ 5 ];

	s_irgchBuffer = ( s_irgchBuffer - 5 * ( ( s_irgchBuffer + 1 ) / 5 ) + 1 );
	auto buf = s_rgtlsBuffers[ s_irgchBuffer ];
	if ( !buf.Get() )
	{
		AUTO_LOCK( g_mutexTempTextBuffer );
		if ( g_bTextBuffersFreed ) Error( "GetPchTempTextBuffer() called after UninitTempTextBuffers()" );
		buf.Set((char*)MemAlloc_Alloc(1024));
		g_vecTempTextBuffer.AddToTail( buf.Get() );
	}

	return buf.Get();
}

size_t GetCchTempTextBuffer()
{
	return 1024;
}

} // namespace GCSDK
