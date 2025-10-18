#include "cbase.h"
#include "uicomponent_mypersona.h"

#include "vgui/ILocalize.h"
#include "bannedwords.h"
#include "gcsdk/gcclient.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

SF_COMPONENT_API_DEF_BEGIN(CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(uint64, GetXuid, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(const wchar_t*, GetName, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(int, IsVacBanned, CUiComponent_MyPersona)
SF_COMPONENT_API_DEF_END(CUiComponent_MyPersona)

UI_COMPONENT_API_DEF_COMMON(CUiComponent_MyPersona, MyPersona)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetXuid)
{
	DevMsg(__FUNCTION__ "\n");

	static char xuidText[256];
	V_snprintf(xuidText, 255, "%llu", s_steamIdMyself);
	pui->Params_SetResult(obj, xuidText);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetName)
{
	DevMsg(__FUNCTION__ "\n");

	auto szName = steamapicontext->SteamFriends()->GetFriendPersonaName(s_steamIdMyself);
	auto szCensored = g_BannedWords.CensorExternalString(s_steamIdMyself.ConvertToUint64(), szName);

	static wchar_t safe_wide_name[510] = { 0 };

	wchar_t wszSafeName[510] = { 0 };
	g_pVGuiLocalize->ConvertANSIToUnicode(szCensored, wszSafeName, sizeof(wszSafeName));
	g_pScaleformUI->MakeStringSafe(wszSafeName, safe_wide_name, sizeof(safe_wide_name));

	pui->Params_SetResult(obj, safe_wide_name);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, IsVacBanned)
{
	DevMsg(__FUNCTION__ "\n");

	pui->Params_SetResult(obj, 0);
}

CSteamID CUiComponent_MyPersona::s_steamIdMyself;

CUiComponent_MyPersona::CUiComponent_MyPersona()
{
	DevMsg(__FUNCTION__ "\n");

	s_steamIdMyself = steamapicontext->SteamUser()->GetSteamID();
}
CUiComponent_MyPersona::~CUiComponent_MyPersona()
{
	DevMsg(__FUNCTION__ "\n");
}

void CUiComponent_MyPersona::RequestAccountPrivacySettings()
{
	DevMsg(__FUNCTION__ "\n");
}