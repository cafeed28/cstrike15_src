#include "cbase.h"

#if defined( INCLUDE_SCALEFORM )
#include "basepanel.h"
#include "pausemenuscreen_scaleform.h"

#include "options_scaleform.h"
#include "howtoplaydialog_scaleform.h"
#include "medalstatsdialog_scaleform.h"
#include "leaderboardsdialog_scaleform.h"
#include "HUD/sfhudradar.h"
#include "HUD/sfhudcallvotepanel.h"

#include "clientmode_csnormal.h"
#include "matchmaking/imatchframework.h"
#include "gameui_interface.h"
#include "gameui_util.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

CPauseMenuScreenScaleform* CPauseMenuScreenScaleform::m_pInstance = NULL;

SFUI_BEGIN_GAME_API_DEF
    SF_COMPONENT_HOST_API_DEF(),
    SFUI_DECL_METHOD(BasePanelRunCommand),
    SFUI_DECL_METHOD(IsQueuedMatchmaking),
    SFUI_DECL_METHOD(IsGotvSpectating),
    SFUI_DECL_METHOD(IsMultiplayer),
    SFUI_DECL_METHOD(NeedsInviteFriends),
    SFUI_DECL_METHOD(IsTraining),
    SFUI_DECL_METHOD(GetTeamNumber),
    SFUI_DECL_METHOD(CanMakeSessionPublic),
    SFUI_DECL_METHOD(SwitchTeams),
    SFUI_DECL_METHOD(IsPlayingGunGameProgressive),
    SFUI_DECL_METHOD(CallVote),
    SFUI_DECL_METHOD(OpenPlayerDetailsPanel),
    SFUI_DECL_METHOD(IsWorkshopMap),
    SFUI_DECL_METHOD(ViewMapInWorkshop),
SFUI_END_GAME_API_DEF(CPauseMenuScreenScaleform, PauseMenu);

void CPauseMenuScreenScaleform::BasePanelRunCommand(SCALEFORM_CALLBACK_ARGS_DECL)
{
    char RunCommandStr[1024];
    V_strncpy(&RunCommandStr[0], pui->Params_GetArgAsString(obj, 0), sizeof(RunCommandStr));

    if (pui->Params_GetNumArgs(obj) >= 2)
    {
        const char* szArg2 = pui->Params_GetArgAsString(obj, 1);
        if (V_stricmp(szArg2, "bCloseMenu") == 0)
        {
            UnloadDialog();
        }
        else if (V_stricmp(szArg2, "bHideMenu") == 0)
        {
            Hide();
        }
        else if (V_stricmp(szArg2, "bReturnToMPGameDialog") == 0)
        {
            BasePanel()->m_bReturnToMPGameMenuOnDisconnect = true;
        }
    }

    // Run slotted command
    {
        char slotnumber[2];
        slotnumber[0] = '0' + GET_ACTIVE_SPLITSCREEN_SLOT();
        slotnumber[1] = 0;
        BasePanel()->PostMessage(BasePanel(), new KeyValues("RunSlottedMenuCommand", "slot", slotnumber, "command", RunCommandStr));
    }
}

SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::IsMultiplayer); // used
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::NeedsInviteFriends); // used
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::GetTeamNumber);
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::CanMakeSessionPublic);
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::SwitchTeams); // used
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::CallVote);
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::OpenPlayerDetailsPanel); // used
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::IsWorkshopMap); // used
SFUI_CALLBACK_TODO(CPauseMenuScreenScaleform::ViewMapInWorkshop); // used

void CPauseMenuScreenScaleform::IsQueuedMatchmaking(SCALEFORM_CALLBACK_ARGS_DECL)
{
    m_pScaleformUI->Params_SetResult(obj, CSGameRules() && CSGameRules()->IsQueuedMatchmaking());
}

void CPauseMenuScreenScaleform::IsTraining(SCALEFORM_CALLBACK_ARGS_DECL)
{
    m_pScaleformUI->Params_SetResult(obj, CSGameRules() && CSGameRules()->IsPlayingTraining());
}
void CPauseMenuScreenScaleform::IsPlayingGunGameProgressive(SCALEFORM_CALLBACK_ARGS_DECL)
{
    m_pScaleformUI->Params_SetResult(obj, CSGameRules() && CSGameRules()->IsPlayingGunGameProgressive());
}

void CPauseMenuScreenScaleform::IsGotvSpectating(SCALEFORM_CALLBACK_ARGS_DECL)
{
    m_pScaleformUI->Params_SetResult(obj, engine->IsHLTV() || engine->IsPlayingDemo());
}

void CPauseMenuScreenScaleform::LoadDialog()
{
    if (!m_pInstance)
    {
        m_pInstance = new CPauseMenuScreenScaleform();
        SFUI_REQUEST_ELEMENT(SF_FULL_SCREEN_SLOT, g_pScaleformUI, CPauseMenuScreenScaleform, m_pInstance, PauseMenu);

        m_pInstance->m_bVisible = true;
        m_pInstance->m_bOwningInput = true;

        GetHud(0).DisableHud();

        engine->ExecuteClientCmd("hidescores");
        engine->ExecuteClientCmd("spec_gui 0");
        ScaleformUI()->DenyInputToGame(true);
        ScaleformUI()->ShowCursor();
    }
    else if (m_pInstance->m_bOwningInput && m_pInstance->m_bUnk)
    {
        m_pInstance->m_bUnk = false;
        m_pInstance->m_bVisible = true;
    }
}

void CPauseMenuScreenScaleform::UnloadDialog()
{
    if (m_pInstance && !m_pInstance->m_bUnk)
    {
        m_pInstance->m_bUnk = true;
        if (!m_pInstance->m_bOwningInput)
        {
            m_pInstance->DestroyDialog();
        }
        else
        {
            m_pInstance->m_bVisible = false;
        }
    }
}

void CPauseMenuScreenScaleform::RestorePanel()
{
    if (m_pInstance && !m_pInstance->m_bVisible)
    {
        m_pInstance->InnerRestorePanel();
    }
}

void CPauseMenuScreenScaleform::InnerRestorePanel(void)
{
    WITH_SLOT_LOCKED
        m_pScaleformUI->Value_InvokeWithoutReturn(m_FlashAPI, "restorePanel", NULL, 0);

    m_bVisible = true;
}

void CPauseMenuScreenScaleform::DestroyDialog()
{
    PrepareUIForPauseMenu(false);
    Hide();
    RemoveFlashElement();

    BasePanel()->PostMessage(BasePanel(), new KeyValues("RunMenuCommand", "command", "ResumeGame"));
    
    if (CSGameRules()->IsPlayingTraining())
    {
        char szTimeScaleCmd[64];
        V_snprintf(szTimeScaleCmd, sizeof(szTimeScaleCmd), "host_timescale %f", m_flTimeScale);
        engine->ClientCmd_Unrestricted(szTimeScaleCmd);
    }
}

void CPauseMenuScreenScaleform::PrepareUIForPauseMenu(bool bUnk)
{
    if (bUnk)
    {
        GetHud(0).DisableHud();
        engine->ExecuteClientCmd("hidescores");
        engine->ExecuteClientCmd("spec_gui 0");

        g_pScaleformUI->DenyInputToGame(true);
        g_pScaleformUI->ShowCursor();
    }
    else
    {
        if (GetHud(0).HudDisabled())
        {
            GetHud(0).EnableHud();
        }

        COptionsScaleform::UnloadDialog();
        CCreateLeaderboardsDialogScaleform::UnloadDialog();
        CCreateMedalStatsDialogScaleform::UnloadDialog();
        CHowToPlayDialogScaleform::UnloadDialog();
        SFHudCallVotePanel::UnloadDialog();
        
        engine->ExecuteClientCmd("spec_gui 1");

        g_pScaleformUI->DenyInputToGame(false);
        g_pScaleformUI->HideCursor();
    }
}

void CPauseMenuScreenScaleform::ShowMenu(bool bShow)
{
    if (bShow && !m_pInstance)
    {
        LoadDialog();
    }
    else
    {
        if (bShow != m_pInstance->m_bVisible)
        {
            if (bShow) m_pInstance->Show();
            else m_pInstance->Hide();
        }
    }
}

void CPauseMenuScreenScaleform::Show()
{
    SF_FORCE_SPLITSCREEN_PLAYER_GUARD(m_iSplitScreenSlot);

    if (!m_bOwningInput)
    {
        if (FlashAPIIsValid())
        {
            WITH_SLOT_LOCKED
                g_pScaleformUI->Value_InvokeWithoutReturn(m_FlashAPI, "showPanel", NULL, 0);
        }
        else
        {
            m_bOwningInput = true;
            SFUI_REQUEST_ELEMENT(SF_FULL_SCREEN_SLOT, g_pScaleformUI, CPauseMenuScreenScaleform, this, PauseMenu);
            
            GetHud(0).DisableHud();
            
            engine->ExecuteClientCmd("hidescores");
            engine->ExecuteClientCmd("spec_gui 0");
            g_pScaleformUI->DenyInputToGame(true);
            g_pScaleformUI->ShowCursor();
        }
    }
    m_bVisible = true;
}

void CPauseMenuScreenScaleform::Hide()
{
    if (m_bFlashAPIIsValid && !m_bOwningInput)
    {
        WITH_SLOT_LOCKED
            g_pScaleformUI->Value_InvokeWithoutReturn(m_FlashAPI, "hidePanel", NULL, 0);

        GetHud().ResetHUD();
        if (GetHud().HudDisabled())
        {
            GetHud().EnableHud();
        }

        SFHudRadar* pPanel = GET_HUDELEMENT(SFHudRadar);
        if (pPanel)
            pPanel->ResizeHud();
    }
    m_bVisible = false;
}

CPauseMenuScreenScaleform::CPauseMenuScreenScaleform() :
    m_bVisible(false),
    m_bOwningInput(false),
    m_bUnk(false),
    m_flTimeScale(1.0f)
{
    m_iSplitScreenSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

    g_pMatchFramework->GetEventsSubscription()->Subscribe(this);
}

CPauseMenuScreenScaleform::~CPauseMenuScreenScaleform()
{
    g_pMatchFramework->GetEventsSubscription()->Unsubscribe(this);
}

void CPauseMenuScreenScaleform::FlashLoaded()
{
}

void CPauseMenuScreenScaleform::FlashReady()
{
    GAMEUI_ACTIVE_SPLITSCREEN_PLAYER_GUARD(m_iSplitScreenSlot);

    IPlayerLocal* pLocalPlayer = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer(XBX_GetActiveUserId());

    WITH_SFVALUEARRAY(args, 1)
    {
        if (pLocalPlayer)
        {
            m_pScaleformUI->ValueArray_SetElement(args, 0, pLocalPlayer->GetName());
        }
        else
        {
            m_pScaleformUI->ValueArray_SetElement(args, 0, "Player1");
        }
        m_pScaleformUI->Value_InvokeWithoutReturn(m_FlashAPI, "SetPlayerName", args, 1);
    }
    
    ListenForGameEvent("cs_match_end_restart");
    ListenForGameEvent("cs_game_disconnected");

    if (m_bUnk)
    {
        DestroyDialog();
    }
    else
    {
        m_bOwningInput = false;
        m_bVisible ? Show() : Hide();
    }
}

void CPauseMenuScreenScaleform::PostUnloadFlash()
{
    StopListeningForAllEvents();
    m_pInstance = nullptr;
    delete this;
}

void CPauseMenuScreenScaleform::OnEvent(KeyValues* event)
{
    SF_COMPONENT_FORWARD_EVENT(event->GetName());
}

void CPauseMenuScreenScaleform::FireGameEvent(IGameEvent* event)
{
    GAMEUI_ACTIVE_SPLITSCREEN_PLAYER_GUARD(m_iSplitScreenSlot);
    if (!V_strcmp(event->GetName(), "cs_game_disconnected"))
    {
        UnloadDialog();
    }
}

#endif // INCLUDE_SCALEFORM