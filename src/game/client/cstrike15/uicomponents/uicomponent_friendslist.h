#pragma once

#include "uicomponent_common.h"
#include "ixboxsystem.h"

class CUiComponent_FriendsList : public CUiComponentGlobalInstanceHelper<CUiComponent_FriendsList> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_FriendsList);

public:
	SF_COMPONENT_FUNCTION(void, GetFriendName);

	SF_COMPONENT_FUNCTION(void, GetXuidFromFriendCode); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendStatus); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendStatusBucket); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendRelationship); // TODO
	SF_COMPONENT_FUNCTION(void, IsFriendPlayingCSGO); // TODO
	SF_COMPONENT_FUNCTION(void, IsFriendJoinable); // TODO
	SF_COMPONENT_FUNCTION(void, IsFriendWatchable); // TODO
	SF_COMPONENT_FUNCTION(void, IsFriendInvited); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendIsVacBanned); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendCompetitiveRank); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendCompetitiveWins); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendCommendations); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendMedalRankByType); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendLevel); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendXp); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendClanId); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendClanTag); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendClanName); // TODO
	SF_COMPONENT_FUNCTION(void, ActionInviteFriend); // TODO
	SF_COMPONENT_FUNCTION(void, ActionShowCSGOProfile); // TODO
	SF_COMPONENT_FUNCTION(void, ActionJoinFriendSession); // TODO
	SF_COMPONENT_FUNCTION(void, ActionWatchFriendSession); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefCount); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefByIndex); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendDisplayItemDefFeatured); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamName); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamTag); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendOfficialTeamFlag); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsCount); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsXuidByIdx); // TODO
	SF_COMPONENT_FUNCTION(void, GetFriendRequestsNotificationNumber); // TODO
	SF_COMPONENT_FUNCTION(void, ToggleMute); // TODO
	SF_COMPONENT_FUNCTION(void, IsSelectedPlayerMuted); // TODO
	SF_COMPONENT_FUNCTION(void, IsLocalPlayerPlayingMatch); // TODO
	SF_COMPONENT_FUNCTION(void, GetPlayerCount); // TODO
	SF_COMPONENT_FUNCTION(void, IsGameInWarmup); // TODO
	SF_COMPONENT_FUNCTION(void, IsGamePaused); // TODO
	SF_COMPONENT_FUNCTION(void, GetCount); // TODO
	SF_COMPONENT_FUNCTION(void, GetXuidByIndex); // TODO

protected:
	const char* GetPlayerNameInternal(XUID xuid);

private:
	FIF_RE_TODO(ActionInviteFriend);
	FIF_RE_TODO(ActionJoinFriendSession);
	FIF_RE_TODO(ActionShowCSGOProfile);
	FIF_RE_TODO(ActionWatchFriendSession);
	FIF_RE_TODO(GetFriendClanTag);
	FIF_RE_TODO(GetFriendCommendations);
	FIF_RE_TODO(GetFriendCompetitiveRank);
	FIF_RE_TODO(GetFriendCompetitiveWins);
	FIF_RE_TODO(GetFriendDisplayItemDefByIndex);
	FIF_RE_TODO(GetFriendDisplayItemDefCount);
	FIF_RE_TODO(GetFriendDisplayItemDefFeatured);
	FIF_RE_TODO(GetFriendIsVacBanned);
	FIF_RE_TODO(GetFriendLevel);
	FIF_RE_TODO(GetFriendMedalRankByType);
	FIF_RE_TODO(GetFriendOfficialTeamFlag);
	FIF_RE_TODO(GetFriendOfficialTeamName);
	FIF_RE_TODO(GetFriendOfficialTeamTag);
	FIF_RE_TODO(GetFriendRelationship);
	FIF_RE_TODO(GetFriendStatus);
	FIF_RE_TODO(GetFriendStatusBucket);
	FIF_RE_TODO(GetFriendXp);
	FIF_RE_TODO(GetPlayerCount);
	FIF_RE_TODO(GetPlayerName);
	FIF_RE_TODO(GetXuidFromFriendCode);
	FIF_RE_TODO(HelperFriendsListSortFunc);
	FIF_RE_TODO(InviteFriend);
	FIF_RE_TODO(IsFriendInvited);
	FIF_RE_TODO(IsFriendJoinable);
	FIF_RE_TODO(IsFriendPlayingCSGO);
	FIF_RE_TODO(IsFriendWatchable);
	FIF_RE_TODO(IsSelectedPlayerMuted);
	FIF_RE_TODO(OnEvent);
	FIF_RE_TODO(RebuildFriendsList);
	FIF_RE_TODO(ReportInviteMessage);
	FIF_RE_TODO(RequestFriendProfileUpdate);
	FIF_RE_TODO(ToggleMute);
};