#include "cbase.h"
#include "uicomponent_friendslist.h"
#include "uicomponent_mypersona.h"

#include "cdll_client_int.h"
#include "c_playerresource.h"
#include "matchmaking/imatchframework.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

SF_COMPONENT_API_DEF_BEGIN(CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetXuidFromFriendCode, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendName, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendStatus, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendStatusBucket, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendRelationship, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsFriendPlayingCSGO, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsFriendJoinable, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsFriendWatchable, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsFriendInvited, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendIsVacBanned, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendCompetitiveRank, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendCompetitiveWins, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendCommendations, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendMedalRankByType, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendLevel, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendXp, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendClanId, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendClanTag, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendClanName, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, ActionInviteFriend, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, ActionShowCSGOProfile, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, ActionJoinFriendSession, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, ActionWatchFriendSession, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendDisplayItemDefCount, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendDisplayItemDefByIndex, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendDisplayItemDefFeatured, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendOfficialTeamName, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendOfficialTeamTag, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendOfficialTeamFlag, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendRequestsCount, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendRequestsXuidByIdx, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetFriendRequestsNotificationNumber, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, ToggleMute, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsSelectedPlayerMuted, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsLocalPlayerPlayingMatch, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetPlayerCount, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsGameInWarmup, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, IsGamePaused, CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetCount, /*CUiComponent_UsersListBase*/ CUiComponent_FriendsList)
	SF_COMPONENT_FUNCTION_API_DEF(void, GetXuidByIndex, /*CUiComponent_UsersListBase*/ CUiComponent_FriendsList)
SF_COMPONENT_API_DEF_END(CUiComponent_FriendsList)

UI_COMPONENT_API_DEF_COMMON(CUiComponent_FriendsList, FriendsList)

static CSteamID s_steamIdMyself;

SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetXuidFromFriendCode)

const char* CUiComponent_FriendsList::GetPlayerNameInternal(XUID xuid)
{
	const char* szName = NULL;

	if (engine->IsConnected() && g_PR)
	{
		for (int playerIndex = 1; playerIndex <= MAX_PLAYERS; playerIndex++)
		{
			if (!g_PR->IsConnected(playerIndex) || g_PR->GetXuid(playerIndex) != xuid)
				continue;

			if (g_PR->IsFakePlayer(playerIndex))
			{
				szName = "BOT";
			}
			else
			{
				szName = g_PR->GetPlayerName(playerIndex);
			}

			break;
		}
	}

	if (!szName)
	{
		auto pPlayer = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetFriendByXUID(xuid);
		if (!pPlayer || !(szName = pPlayer->GetName()))
		{
			szName = NULL;
			if (!steamapicontext->SteamFriends()->RequestUserInformation(xuid, true))
			{
				szName = steamapicontext->SteamFriends()->GetFriendPersonaName(xuid);
			}
		}
	}

	return g_BannedWords.CensorExternalString(xuid, szName ? szName : "");
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_FriendsList, GetFriendName)
{
	SF_INTEGRATION_XUID_PARAM(xuidFriend, 0);

	auto szName = GetPlayerNameInternal(xuidFriend);

	static wchar_t safe_wide_name[510] = { 0 };

	wchar_t wszSafeName[510] = { 0 };
	g_pVGuiLocalize->ConvertANSIToUnicode(szName, wszSafeName, sizeof(wszSafeName));
	g_pScaleformUI->MakeStringSafe(wszSafeName, safe_wide_name, sizeof(safe_wide_name));

	pui->Params_SetResult(obj, safe_wide_name);
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendStatus)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendStatusBucket)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendRelationship)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsFriendPlayingCSGO)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsFriendJoinable)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsFriendWatchable)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsFriendInvited)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendIsVacBanned)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_FriendsList, GetFriendCompetitiveRank)
{
	if (pui->Params_GetArgType(obj, 0) == IUIMarshalHelper::VT_String)
	{
		pui->Params_SetResult(obj, GetFriendCompetitiveRank(pui->Params_GetArgAsString(obj, 0)));
	}
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_FriendsList, GetFriendCompetitiveWins)
{
	if (pui->Params_GetArgType(obj, 0) == IUIMarshalHelper::VT_String)
	{
		const char* pchSteamID = pui->Params_GetArgAsString(obj, 0);
		pui->Params_SetResult(obj, GetFriendCompetitiveWins(pchSteamID));
	}
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_FriendsList, GetFriendCommendations)
{
	if (pui->Params_GetArgType(obj, 0) == IUIMarshalHelper::VT_String)
	{
		const char* pchSteamID = pui->Params_GetArgAsString(obj, 0);
		const char* szCommendations = pui->Params_GetArgType(obj, 1) == IUIMarshalHelper::VT_String ? pui->Params_GetArgAsString(obj, 1) : NULL;

		pui->Params_SetResult(obj, GetFriendCommendations(pchSteamID, szCommendations));
	}
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendMedalRankByType)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendLevel)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendXp)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendClanId)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendClanTag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendClanName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, ActionInviteFriend)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, ActionShowCSGOProfile)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, ActionJoinFriendSession)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, ActionWatchFriendSession)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendDisplayItemDefCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendDisplayItemDefByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendDisplayItemDefFeatured)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendOfficialTeamName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendOfficialTeamTag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendOfficialTeamFlag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendRequestsCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendRequestsXuidByIdx)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetFriendRequestsNotificationNumber)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, ToggleMute)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsSelectedPlayerMuted)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsLocalPlayerPlayingMatch)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetPlayerCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsGameInWarmup)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, IsGamePaused)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_FriendsList, GetXuidByIndex)

int CUiComponent_FriendsList::GetFriendCommendations(const char* pchSteamID, const char* szCommendation)
{
	CSteamID steamID(pchSteamID);
	//CUiComponent_PartyList::CheckMyPersonaDataInPartyList(steamID);

	if (!szCommendation || !*szCommendation)
		return 0;

	if (steamID == s_steamIdMyself)
	{
		return CUiComponent_MyPersona::GetInstance()->GetCommendations(szCommendation);
	}

	AssertOnce(false); // RE TODO
}

int CUiComponent_FriendsList::GetFriendCompetitiveRank(const char* pchSteamID)
{
	CSteamID steamID(pchSteamID);
	if (steamID == s_steamIdMyself)
	{
		return CUiComponent_MyPersona::GetInstance()->GetCompetitiveRank();
	}

	AssertOnce(false); // RE TODO
}

int CUiComponent_FriendsList::GetFriendCompetitiveWins(const char* pchSteamID)
{
	CSteamID steamID(pchSteamID);
	if (steamID == s_steamIdMyself)
	{
		return CUiComponent_MyPersona::GetInstance()->GetCompetitiveWins();
	}

	AssertOnce(false); // RE TODO
}

CUiComponent_FriendsList::CUiComponent_FriendsList()
{
	s_steamIdMyself = steamapicontext->SteamUser()->GetSteamID();
}

CUiComponent_FriendsList::~CUiComponent_FriendsList()
{
	DevMsg(__FUNCTION__ "\n");
}