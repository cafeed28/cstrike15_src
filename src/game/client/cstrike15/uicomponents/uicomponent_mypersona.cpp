#include "cbase.h"
#include "uicomponent_mypersona.h"
#include "uicomponent_friendslist.h"
#include "components/scaleformcomponent_common.h"

#include "vgui/ILocalize.h"
#include "bannedwords.h"
#include "gcsdk/gcclient.h"
#include "cstrike15_item_inventory.h"
#include "econ_game_account_client.h"
#include "gc_clientsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

SF_COMPONENT_API_DEF_BEGIN(CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(XUID, GetXuid, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(const wchar_t*, GetName, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetFriendCode, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetLauncherType, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetLicenseType, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(bool, IsInventoryValid, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(int, IsVacBanned, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetCompetitiveRank, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetCompetitiveWins, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCommendations, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyMedalRankByType, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyMedalAdditionalInfo, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyDisplayItemDefCount, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyDisplayItemDefByIndex, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyDisplayItemDefFeatured, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyClanCount, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyClanIdByIndex, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyClanTagById, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyClanNameById, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyClanUsersById, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyOfficialTeamID, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyOfficialTeamName, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyOfficialTeamTag, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyOfficialTeamFlag, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyOfficialTournamentName, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCurrentXp, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCurrentLevel, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, HasPrestige, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetXpPerLevel, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetActiveXpBonuses, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTimePlayedConsecutively, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetMyNotifications, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionAcknowledgeNotifications, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetElevatedState, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetElevatedTime, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionElevate, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionClientAuthKeyCode, CUiComponent_MyPersona)
	SF_COMPONENT_FUNCTION_API_DEF(TODO, GetClientAuthKeyCode, CUiComponent_MyPersona)
SF_COMPONENT_API_DEF_END(CUiComponent_MyPersona)

UI_COMPONENT_API_DEF_COMMON(CUiComponent_MyPersona, MyPersona)

static CSteamID s_steamIdMyself;
extern CMsgGCCStrike15_v2_MatchmakingGC2ClientHello g_GC2ClientHello;

bool Helper_MyPersonaLauncherPerfectWorld()
{
	static bool s_bPerfectWorld = !!CommandLine()->FindParm("-perfectworld");
	return s_bPerfectWorld;
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetXuid)
{
	static char xuidText[256];
	V_snprintf(xuidText, 255, "%llu", s_steamIdMyself);
	pui->Params_SetResult(obj, xuidText);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetName)
{
	auto szName = steamapicontext->SteamFriends()->GetFriendPersonaName(s_steamIdMyself);
	auto szCensored = g_BannedWords.CensorExternalString(s_steamIdMyself.ConvertToUint64(), szName);

	static wchar_t safe_wide_name[510] = { 0 };

	wchar_t wszSafeName[510] = { 0 };
	g_pVGuiLocalize->ConvertANSIToUnicode(szCensored, wszSafeName, sizeof(wszSafeName));
	g_pScaleformUI->MakeStringSafe(wszSafeName, safe_wide_name, sizeof(safe_wide_name));

	pui->Params_SetResult(obj, safe_wide_name);
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetFriendCode)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetLauncherType)
{
	pui->Params_SetResult(obj, Helper_MyPersonaLauncherPerfectWorld() ? "perfectworld" : "steam");
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetLicenseType)
{
	const char* szResult = "none";

	if (steamapicontext->SteamApps()->BIsSubscribedFromFreeWeekend())
	{
		szResult = "free";
	}
	else if (steamapicontext->SteamApps()->BIsSubscribed())
	{
		szResult = steamapicontext->SteamApps()->GetAppOwner() == s_steamIdMyself ? "purchased" : "shared";
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, IsInventoryValid)
{
	//pui->Params_SetResult(obj, m_bIsInventoryValid);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, IsVacBanned)
{
	pui->Params_SetResult(obj, (int)IsVacBanned());
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetCompetitiveRank)
{
	pui->Params_SetResult(obj, GetCompetitiveRank());
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetCompetitiveWins)
{
	pui->Params_SetResult(obj, GetCompetitiveWins());
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCommendations)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyMedalRankByType)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyMedalAdditionalInfo)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyDisplayItemDefFeatured)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetMyClanCount)
{
	//pui->Params_SetResult(obj, GetMyClanCount());
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanIdByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanTagById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanNameById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyClanUsersById)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamID)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamTag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTeamFlag)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetMyOfficialTournamentName)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCurrentXp)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetCurrentLevel)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, HasPrestige)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetXpPerLevel)
{
	pui->Params_SetResult(obj, 5000);
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetActiveXpBonuses)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetTimePlayedConsecutively)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, GetMyNotifications)
{
	pui->Params_SetResult(obj, GetMyNotifications());
}


SF_COMPONENT_FUNCTION_IMPL(CUiComponent_MyPersona, ActionAcknowledgeNotifications)
{
	ActionAcknowledgeNotifications();
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetElevatedState)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetElevatedTime)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, ActionElevate)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, ActionClientAuthKeyCode)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_MyPersona, GetClientAuthKeyCode)

//

CUiComponent_MyPersona::CUiComponent_MyPersona()
: m_CallbackPersonaStateChange(this, &CUiComponent_MyPersona::Steam_OnPersonaStateChange)
{
	s_steamIdMyself = steamapicontext->SteamUser()->GetSteamID();

	g_pMatchFramework->GetEventsSubscription()->Subscribe(this);
}

CUiComponent_MyPersona::~CUiComponent_MyPersona()
{

}

void CUiComponent_MyPersona::OnEvent(KeyValues* event)
{

}

void CUiComponent_MyPersona::Steam_OnPersonaStateChange(PersonaStateChange_t * pParam)
{
	if ((pParam->m_nChangeFlags & k_EPersonaChangeName) && s_steamIdMyself == pParam->m_ulSteamID)
	{
		UI_COMPONENT_BROADCAST_EVENT(MyPersona, NameChanged, pParam);
	}

	if ((pParam->m_nChangeFlags & (k_EPersonaChangeRelationshipChanged | k_EPersonaChangeName)))
	{
		CFmtStrN<256> fmtStr("%llu", pParam->m_ulSteamID);
		UI_COMPONENT_BROADCAST_EVENT(FriendsList, NameChanged, fmtStr.Get());
	}

	if ((pParam->m_nChangeFlags & (k_EPersonaChangeComeOnline | k_EPersonaChangeGoneOffline | k_EPersonaChangeRelationshipChanged)))
		m_flPersonaStateChangeTime = Plat_FloatTime();
}

//

uint32 CUiComponent_MyPersona::IsVacBanned()
{
	uint32 vac_banned = g_GC2ClientHello.vac_banned();
	if (vac_banned != 0 && g_GC2ClientHello.penalty_reason() == 0)
		return vac_banned;
	return 0;
}

int CUiComponent_MyPersona::GetCommendations(const char* szCommendation)
{
	if (!szCommendation || !*szCommendation ||! g_GC2ClientHello.has_commendation())
		return 0;

	if (!V_stricmp("friendly", szCommendation))
	{
		return g_GC2ClientHello.commendation().cmd_friendly();
	}
	else if (!V_stricmp("teaching", szCommendation))
	{
		return g_GC2ClientHello.commendation().cmd_teaching();
	}
	else if (!V_stricmp("leader", szCommendation))
	{
		return g_GC2ClientHello.commendation().cmd_leader();
	}

	return 0;
}

int CUiComponent_MyPersona::GetCompetitiveRank()
{
	if (g_GC2ClientHello.has_ranking())
	{
		return g_GC2ClientHello.ranking().rank_id();
	}
	return 0;
}

int CUiComponent_MyPersona::GetCompetitiveWins()
{
	if (g_GC2ClientHello.has_ranking())
	{
		return g_GC2ClientHello.ranking().wins();
	}
	return 0;
}

extern CEconGameAccountClient* GetSOCacheGameAccountClient(GCSDK::CGCClientSharedObjectCache* pSOCache);

void CUiComponent_MyPersona::ActionAcknowledgeNotifications()
{
	CPlayerInventory* pLocalInventory = CSInventoryManager()->GetLocalInventory();
	if (pLocalInventory)
	{
		CEconGameAccountClient* pGameAccountClient = GetSOCacheGameAccountClient(pLocalInventory->GetSOC());
		if (pGameAccountClient)
		{
			if (pGameAccountClient->Obj().bonus_xp_usedflags() & 0xC)
			{
				pGameAccountClient->Obj().set_bonus_xp_usedflags(pGameAccountClient->Obj().bonus_xp_usedflags() & ~0xC);

				GCSDK::CProtoBufMsg<CMsgGCCStrike15_v2_AcknowledgePenalty> msg(k_EMsgGCCStrike15_v2_AcknowledgePenalty);
				msg.Body().set_acknowledged(2);

				GCClientSystem()->BSendMessage(msg);

				UI_COMPONENT_BROADCAST_EVENT(GC, Hello);
			}
			else if (pGameAccountClient->Obj().bonus_xp_usedflags() & 0x10000000)
			{
				UI_COMPONENT_BROADCAST_EVENT(GC, Hello);
			}
		}
	}
}

const char* CUiComponent_MyPersona::GetMyNotifications()
{
	CPlayerInventory* pLocalInventory = CSInventoryManager()->GetLocalInventory();
	if (pLocalInventory)
	{
		CEconGameAccountClient* pGameAccountClient = GetSOCacheGameAccountClient(pLocalInventory->GetSOC());
		if (pGameAccountClient)
		{
			uint32 nXpBonusFlags = pGameAccountClient->ComputeXpBonusFlagsNow();

			static CUtlStringBuilder strActiveBonuses;
			if (nXpBonusFlags & 8)
			{
				strActiveBonuses.AppendFormat("6,");
			}
			else if (nXpBonusFlags & 4)
			{
				strActiveBonuses.AppendFormat("5,");
			}

			if (nXpBonusFlags & 0x10000000)
			{
				if (m_bUnk2)
					strActiveBonuses.AppendFormat("4,");
			}
			else if (!m_bUnk2)
			{
				m_bUnk2 = true;
			}

			return strActiveBonuses.Access();
		}
	}

	return NULL;
}