#pragma once

#include "uicomponent_common.h"
#include "matchmaking/imatchframework.h"

class CUiComponent_MyPersona : public CUiComponentGlobalInstanceHelper<CUiComponent_MyPersona>, public IMatchEventsSink {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_MyPersona);

public:
	SF_COMPONENT_FUNCTION(XUID, GetXuid);
	SF_COMPONENT_FUNCTION(const wchar_t*, GetName);
	SF_COMPONENT_FUNCTION(void, GetFriendCode);
	SF_COMPONENT_FUNCTION(const char*, GetLauncherType);
	SF_COMPONENT_FUNCTION(const char*, GetLicenseType);
	SF_COMPONENT_FUNCTION(bool, IsInventoryValid);
	SF_COMPONENT_FUNCTION(int, IsVacBanned);
	SF_COMPONENT_FUNCTION(int, GetCompetitiveRank);
	SF_COMPONENT_FUNCTION(int, GetCompetitiveWins);
	SF_COMPONENT_FUNCTION(void, GetCommendations);
	SF_COMPONENT_FUNCTION(void, GetMyMedalRankByType);
	SF_COMPONENT_FUNCTION(void, GetMyMedalAdditionalInfo);
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefCount);
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefByIndex);
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefFeatured);
	SF_COMPONENT_FUNCTION(int, GetMyClanCount);
	SF_COMPONENT_FUNCTION(void, GetMyClanIdByIndex);
	SF_COMPONENT_FUNCTION(void, GetMyClanTagById);
	SF_COMPONENT_FUNCTION(void, GetMyClanNameById);
	SF_COMPONENT_FUNCTION(void, GetMyClanUsersById);
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamID);
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamName);
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamTag);
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamFlag);
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTournamentName);
	SF_COMPONENT_FUNCTION(void, GetCurrentXp);
	SF_COMPONENT_FUNCTION(void, GetCurrentLevel);
	SF_COMPONENT_FUNCTION(void, HasPrestige);
	SF_COMPONENT_FUNCTION(int, GetXpPerLevel);
	SF_COMPONENT_FUNCTION(void, GetActiveXpBonuses);
	SF_COMPONENT_FUNCTION(void, GetTimePlayedConsecutively);
	SF_COMPONENT_FUNCTION(const char*, GetMyNotifications);
	SF_COMPONENT_FUNCTION(void, ActionAcknowledgeNotifications);
	SF_COMPONENT_FUNCTION(void, GetElevatedState);
	SF_COMPONENT_FUNCTION(void, GetElevatedTime);
	SF_COMPONENT_FUNCTION(void, ActionElevate);
	SF_COMPONENT_FUNCTION(void, ActionClientAuthKeyCode);
	SF_COMPONENT_FUNCTION(void, GetClientAuthKeyCode);

public:
	FIF_RE_TODO(RequestAccountPrivacySettings);
	
	void OnEvent(KeyValues* event) override;

	uint32 IsVacBanned();

	int GetCommendations(const char* szCommendation);
	int GetCompetitiveRank();
	int GetCompetitiveWins();

	FIF_RE_TODO(ActionClientAuthKeyCode);
	FIF_RE_TODO(ActionElevate);
	FIF_RE_TODO(GetActiveXpBonuses);
	FIF_RE_TODO(GetCurrentLevel);
	FIF_RE_TODO(GetCurrentXp);
	FIF_RE_TODO(GetElevatedTime);
	FIF_RE_TODO(GetFriendCode);
	FIF_RE_TODO(GetLicenseType);
	FIF_RE_TODO(GetLocalPlayerAccountElevationState);
	FIF_RE_TODO(GetMyClanCount);
	FIF_RE_TODO(GetMyClanIdByIndex);
	FIF_RE_TODO(GetMyClanNameById);
	FIF_RE_TODO(GetMyClanTagById);
	FIF_RE_TODO(GetMyDisplayItemDefByIndex);
	FIF_RE_TODO(GetMyDisplayItemDefCount);
	FIF_RE_TODO(GetMyDisplayItemDefFeatured);
	FIF_RE_TODO(GetMyMedalAdditionalInfoInternal);
	FIF_RE_TODO(GetMyMedalRankByType);
	FIF_RE_TODO(GetMyMedalRankByTypeIndex);
	FIF_RE_TODO(GetMyOfficialTeamFlag);
	FIF_RE_TODO(GetMyOfficialTeamName);
	FIF_RE_TODO(GetMyOfficialTeamTag);
	FIF_RE_TODO(LocalPlayerIsPrimeEligible);
	FIF_RE_TODO(OnMyPersonaInventoryUpdatedImmediate);
	FIF_RE_TODO(RememberClientAuthKeyCode);
	FIF_RE_TODO(RequestAccountPrivacySettingsChange);

private:
	void ActionAcknowledgeNotifications();
	const char* GetMyNotifications();

private:
	STEAM_CALLBACK(CUiComponent_MyPersona, Steam_OnPersonaStateChange, PersonaStateChange_t, m_CallbackPersonaStateChange);

	struct CachedClanInfo_t
	{
		CSteamID steamID;
		int nOnline;
		int nInGame;
		int nChatting;
		char pad[20];
	};

	PlayerMedalsInfo* m_pMsgPlayersMedalInfo;
	double m_flMedalsChangedTime;
	double m_flGCHelloTime;
	double m_flPersonaStateChangeTime;
	AccountActivity* m_pMsgAccountActivity;
	double m_flUnk1;
	bool m_bIsInventoryValid;
	bool m_bUnk0;
	bool m_bUnk1;
	bool m_bUnk2;
	bool m_bUnk3;
	double m_flUnk2;
	CUtlMap<unsigned int, CUtlString, int> m_map;
	CUtlVector<CachedClanInfo_t> m_vecCachedClanInfo;
};