#pragma once

#include "uicomponent_common.h"

class CUiComponent_MyPersona : public CUiComponentGlobalInstanceHelper<CUiComponent_MyPersona> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_MyPersona);

public:
	SF_COMPONENT_FUNCTION(uint64, GetXuid);
	SF_COMPONENT_FUNCTION(const wchar_t*, GetName);
	SF_COMPONENT_FUNCTION(bool, IsVacBanned);

	SF_COMPONENT_FUNCTION(void, ActionAcknowledgeNotifications); // TODO
	SF_COMPONENT_FUNCTION(void, ActionClientAuthKeyCode); // TODO
	SF_COMPONENT_FUNCTION(void, ActionElevate); // TODO
	SF_COMPONENT_FUNCTION(void, GetActiveXpBonuses); // TODO
	SF_COMPONENT_FUNCTION(void, GetClientAuthKeyCode); // TODO
	SF_COMPONENT_FUNCTION(void, GetCommendations); // TODO
	SF_COMPONENT_FUNCTION(void, GetCompetitiveRank); // TODO
	SF_COMPONENT_FUNCTION(void, GetCompetitiveWins); // TODO
	SF_COMPONENT_FUNCTION(void, GetCurrentLevel); // TODO
	SF_COMPONENT_FUNCTION(void, GetCurrentXp); // TODO
	SF_COMPONENT_FUNCTION(void, GetElevatedState); // TODO
	SF_COMPONENT_FUNCTION(void, GetElevatedTime); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendCode); // TODO
	SF_COMPONENT_FUNCTION(void, GetLauncherType); // TODO
	SF_COMPONENT_FUNCTION(void, GetLicenseType); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyClanCount); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyClanIdByIndex); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyClanNameById); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyClanTagById); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyClanUsersById); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefByIndex); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefCount); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyDisplayItemDefFeatured); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyMedalAdditionalInfo); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyMedalRankByType); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyNotifications); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamFlag); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamID); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamName); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTeamTag); // TODO
	SF_COMPONENT_FUNCTION(void, GetMyOfficialTournamentName); // TODO
	SF_COMPONENT_FUNCTION(void, GetTimePlayedConsecutively); // TODO
	SF_COMPONENT_FUNCTION(void, GetXpPerLevel); // TODO
	SF_COMPONENT_FUNCTION(void, HasPrestige); // TODO
	SF_COMPONENT_FUNCTION(void, IsInventoryValid); // TODO

private:
	static CSteamID s_steamIdMyself;

private:
	FIF_RE_TODO(ActionAcknowledgeNotifications);
	FIF_RE_TODO(ActionClientAuthKeyCode);
	FIF_RE_TODO(ActionElevate);
	FIF_RE_TODO(GetActiveXpBonuses);
	FIF_RE_TODO(GetCommendations);
	FIF_RE_TODO(GetCompetitiveRank);
	FIF_RE_TODO(GetCompetitiveWins);
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
	FIF_RE_TODO(GetMyNotifications);
	FIF_RE_TODO(GetMyOfficialTeamFlag);
	FIF_RE_TODO(GetMyOfficialTeamName);
	FIF_RE_TODO(GetMyOfficialTeamTag);
	FIF_RE_TODO(IsVacBanned);
	FIF_RE_TODO(LocalPlayerIsPrimeEligible);
	FIF_RE_TODO(OnEvent);
	FIF_RE_TODO(OnMyPersonaInventoryUpdatedImmediate);
	FIF_RE_TODO(RememberClientAuthKeyCode);
	FIF_RE_TODO(RequestAccountPrivacySettings);
	FIF_RE_TODO(RequestAccountPrivacySettingsChange);
	FIF_RE_TODO(Steam_OnPersonaStateChange);
};