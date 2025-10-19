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

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, ActionAcknowledgeNotifications)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, ActionClientAuthKeyCode)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, ActionElevate)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetActiveXpBonuses)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetClientAuthKeyCode)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCommendations)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCompetitiveRank)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCompetitiveWins)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCurrentLevel)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCurrentXp)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetElevatedState)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetElevatedTime)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetFriendCode)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetLauncherType)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetLicenseType)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanIdByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanNameById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanTagById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanUsersById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefFeatured)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyMedalAdditionalInfo)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyMedalRankByType)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyNotifications)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamFlag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamID)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamTag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTournamentName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetTimePlayedConsecutively)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetXpPerLevel)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, HasPrestige)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, IsInventoryValid)

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
