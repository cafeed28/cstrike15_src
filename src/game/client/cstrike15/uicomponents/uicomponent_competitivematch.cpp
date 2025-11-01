#include "cbase.h"
#include "uicomponent_competitivematch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

extern CMsgGCCStrike15_v2_MatchmakingGC2ClientHello g_GC2ClientHello;

SF_COMPONENT_API_DEF_BEGIN(CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(bool, HasOngoingMatch, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionReconnectToOngoingMatch, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionAbandonOngoingMatch, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionMatchmaking, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCooldownSecondsRemaining, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCooldownType, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetCooldownReason, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, ActionAcknowledgePenalty, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamCount, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamNameByIndex, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamTagByIndex, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamFlagByIndex, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamNameByID, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamTagByID, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentTeamFlagByID, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentStageCount, CUiComponent_CompetitiveMatch)
    SF_COMPONENT_FUNCTION_API_DEF(TODO, GetTournamentStageNameByIndex, CUiComponent_CompetitiveMatch)
SF_COMPONENT_API_DEF_END(CUiComponent_CompetitiveMatch)

UI_COMPONENT_API_DEF_COMMON(CUiComponent_CompetitiveMatch, CompetitiveMatch)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_CompetitiveMatch, HasOngoingMatch)
{
    pui->Params_SetResult(obj, g_GC2ClientHello.has_ongoingmatch());
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, ActionReconnectToOngoingMatch)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, ActionAbandonOngoingMatch)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, ActionMatchmaking)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_CompetitiveMatch, GetCooldownSecondsRemaining)
{
    int nResult = 0;
    if (g_GC2ClientHello.has_penalty_seconds())
    {
        tm tmStruct;
        Plat_GetLocalTime(&tmStruct);

        struct tm tmNow;
        Plat_GetLocalTime(&tmNow);
        time_t tgm = Plat_timegm(&tmNow);
        
        nResult = tgm - g_GC2ClientHello.penalty_seconds();
        if (nResult < 0)
        {
            int penalty_reason = g_GC2ClientHello.penalty_reason();
            if (penalty_reason <= 18)
            {
                long bittest = 0x43;
                if (_bittest(&bittest, penalty_reason - 12)) // RE TODO
                {
                    nResult = 0;
                }
            }
        }

    }
    pui->Params_SetResult(obj, nResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_CompetitiveMatch, GetCooldownType)
{
    int penalty_reason = g_GC2ClientHello.penalty_reason();
    const char* szResult;
    
    long bittest;
    
    bittest = 0x18CD;
    if (penalty_reason <= 20 && _bittest(&bittest, penalty_reason - 8)) // RE TODO
    {
        szResult = "global";
    }
    else
    {
        bittest = 0x43;
        if (penalty_reason > 18 || !_bittest(&bittest, penalty_reason - 12)) // RE TODO
        {
            szResult = "competitive";
        }
        else
        {
            szResult = "green";
        }
    }

    pui->Params_SetResult(obj, szResult);
}

const char* mapCooldownReasons[] = {
    "#SFUI_CooldownExplanationReason_Kicked",
    "#SFUI_CooldownExplanationReason_TK_Limit",
    "#SFUI_CooldownExplanationReason_TK_Spawn",
    "#SFUI_CooldownExplanationReason_DisconnectedTooLong",
    "#SFUI_CooldownExplanationReason_Abandon",
    "#SFUI_CooldownExplanationReason_TH_Limit",
    "#SFUI_CooldownExplanationReason_TH_Spawn",
    "#SFUI_CooldownExplanationReason_OfficialBan",
    "#SFUI_CooldownExplanationReason_KickedTooMuch",
    "#SFUI_CooldownExplanationReason_ConvictedForCheating",
    "#SFUI_CooldownExplanationReason_ConvictedForBehavior",
    "#SFUI_CooldownExplanationReason_Abandon_Grace",
    "#SFUI_CooldownExplanationReason_DisconnectedTooLong_Grace",
    "#SFUI_CooldownExplanationReason_OfficialBan",
    ""
    "#SFUI_CooldownExplanationReason_FailedToConnect",
    "#SFUI_CooldownExplanationReason_KickAbuse",
    "#SFUI_CooldownExplanationReason_SkillGroupCalibration",
    "#SFUI_CooldownExplanationReason_GsltViolation",
    "#SFUI_CooldownExplanationReason_GsltViolation",
};

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_CompetitiveMatch, GetCooldownReason)
{
    int penalty_reason = g_GC2ClientHello.penalty_reason();
    if (penalty_reason < ARRAYSIZE(mapCooldownReasons))
    {
        pui->Params_SetResult(obj, mapCooldownReasons[penalty_reason]);
    }

    Assert(false);
    pui->Params_SetResult(obj, "");
}

SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, ActionAcknowledgePenalty)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamNameByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamTagByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamFlagByIndex)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamNameByID)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamTagByID)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentTeamFlagByID)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentStageCount)
SF_COMPONENT_FUNCTION_TODO(CUiComponent_CompetitiveMatch, GetTournamentStageNameByIndex)

CUiComponent_CompetitiveMatch::CUiComponent_CompetitiveMatch()
{

}

CUiComponent_CompetitiveMatch::~CUiComponent_CompetitiveMatch()
{

}