#pragma once

#include "uicomponent_common.h"
#include "matchmaking/imatchframework.h"

class CUiComponent_CompetitiveMatch : public CUiComponentGlobalInstanceHelper<CUiComponent_CompetitiveMatch> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_CompetitiveMatch);

public:
	SF_COMPONENT_FUNCTION(void, HasOngoingMatch);
	SF_COMPONENT_FUNCTION(void, ActionReconnectToOngoingMatch);
	SF_COMPONENT_FUNCTION(void, ActionAbandonOngoingMatch);
	SF_COMPONENT_FUNCTION(void, ActionMatchmaking);
	SF_COMPONENT_FUNCTION(void, GetCooldownSecondsRemaining);
	SF_COMPONENT_FUNCTION(void, GetCooldownType);
	SF_COMPONENT_FUNCTION(void, GetCooldownReason);
	SF_COMPONENT_FUNCTION(void, ActionAcknowledgePenalty);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamCount);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamNameByIndex);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamTagByIndex);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamFlagByIndex);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamNameByID);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamTagByID);
	SF_COMPONENT_FUNCTION(void, GetTournamentTeamFlagByID);
	SF_COMPONENT_FUNCTION(void, GetTournamentStageCount);
	SF_COMPONENT_FUNCTION(void, GetTournamentStageNameByIndex);

private:
};