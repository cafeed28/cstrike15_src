#pragma once

#include "uicomponent_common.h"
#include "ixboxsystem.h"

class CUiComponent_FriendsList : public CUiComponentGlobalInstanceHelper<CUiComponent_FriendsList> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_FriendsList);

public:
	SF_COMPONENT_FUNCTION(void, GetXuidFromFriendCode);
	SF_COMPONENT_FUNCTION(void, GetFriendName);
	SF_COMPONENT_FUNCTION(void, GetFriendStatus);
	SF_COMPONENT_FUNCTION(void, GetFriendStatusBucket);
	SF_COMPONENT_FUNCTION(void, GetFriendRelationship);
	SF_COMPONENT_FUNCTION(void, IsFriendPlayingCSGO);
	SF_COMPONENT_FUNCTION(void, IsFriendJoinable);
	SF_COMPONENT_FUNCTION(void, IsFriendWatchable);
	SF_COMPONENT_FUNCTION(void, IsFriendInvited);
	SF_COMPONENT_FUNCTION(void, GetFriendIsVacBanned);
	SF_COMPONENT_FUNCTION(void, GetFriendCompetitiveRank);
	SF_COMPONENT_FUNCTION(void, GetFriendCompetitiveWins);
	SF_COMPONENT_FUNCTION(void, GetFriendCommendations);
	SF_COMPONENT_FUNCTION(void, GetFriendMedalRankByType);
	SF_COMPONENT_FUNCTION(void, GetFriendLevel);
	SF_COMPONENT_FUNCTION(void, GetFriendXp);
	SF_COMPONENT_FUNCTION(void, GetFriendClanId);
	SF_COMPONENT_FUNCTION(void, GetFriendClanTag);
	SF_COMPONENT_FUNCTION(void, GetFriendClanName);
	SF_COMPONENT_FUNCTION(void, ActionInviteFriend);
	SF_COMPONENT_FUNCTION(void, ActionShowCSGOProfile);
	SF_COMPONENT_FUNCTION(void, ActionJoinFriendSession);
	SF_COMPONENT_FUNCTION(void, ActionWatchFriendSession);
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefCount);
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefByIndex);
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefFeatured);
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamName);
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamTag);
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamFlag);
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsCount);
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsXuidByIdx);
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsNotificationNumber);
	SF_COMPONENT_FUNCTION(void, ToggleMute);
	SF_COMPONENT_FUNCTION(void, IsSelectedPlayerMuted);
	SF_COMPONENT_FUNCTION(void, IsLocalPlayerPlayingMatch);
	SF_COMPONENT_FUNCTION(void, GetPlayerCount);
	SF_COMPONENT_FUNCTION(void, IsGameInWarmup);
	SF_COMPONENT_FUNCTION(void, IsGamePaused);
	SF_COMPONENT_FUNCTION(void, GetCount);
	SF_COMPONENT_FUNCTION(void, GetXuidByIndex);

protected:
	const char* GetPlayerNameInternal(XUID xuid);
};