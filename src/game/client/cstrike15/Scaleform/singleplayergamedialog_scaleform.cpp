#include "cbase.h"

#if defined( INCLUDE_SCALEFORM )
#include "basepanel.h"
#include "singleplayergamedialog_scaleform.h"

#include "matchmaking/imatchframework.h"
#include "gameui_interface.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

CCreateSinglePlayerGameDialogScaleform* CCreateSinglePlayerGameDialogScaleform::m_pInstance = NULL;

SFUI_BEGIN_GAME_API_DEF
SFUI_DECL_METHOD(OnOk),
SFUI_DECL_METHOD(CheckGameSettingsRequirements),
SFUI_DECL_METHOD(SetMatchmakingQuery),
SFUI_DECL_METHOD(SetCustomBotDifficulty),
SFUI_DECL_METHOD(UpdatedSelections),
SFUI_DECL_METHOD(UpdatePendingInvites),
SFUI_DECL_METHOD(ShowInviteOverlay),
SFUI_DECL_METHOD(GetQueuedMatchmakingTime),
SFUI_DECL_METHOD(GetQueuedMatchmakingPlayers),
SFUI_DECL_METHOD(GetQueuedMatchmakingPreferredMaplist),
SFUI_DECL_METHOD(FilterWorkshopMapsByTags),
SFUI_DECL_METHOD(ViewMapInWorkshop),
SFUI_DECL_METHOD(ViewAllMapsInWorkshop),
SFUI_DECL_METHOD(GetWorkshopMapPath),
SFUI_DECL_METHOD(GetWorkshopMapID),
SFUI_DECL_METHOD(GetWorkshopMapDownloadProgress),
SFUI_DECL_METHOD(EnumerateWorkshopMapsFailed),
SFUI_DECL_METHOD(RefreshFileInfo),
SFUI_DECL_METHOD(DownloadCurrentGamesCount),
SFUI_END_GAME_API_DEF(CCreateSinglePlayerGameDialogScaleform, StartSinglePlayer);

void CCreateSinglePlayerGameDialogScaleform::LoadDialog(bool bUsingMatchmaking, bool bTeamLobbyMode, bool bTrainingMode)
{
	if (!m_pInstance)
	{
		m_pInstance = new CCreateSinglePlayerGameDialogScaleform(bUsingMatchmaking, bTeamLobbyMode, bTrainingMode);
		SFUI_REQUEST_ELEMENT(SF_FULL_SCREEN_SLOT, g_pScaleformUI, CCreateSinglePlayerGameDialogScaleform, m_pInstance, StartSinglePlayer);
	}
}

void CCreateSinglePlayerGameDialogScaleform::UnloadDialog()
{
	if (m_pInstance)
	{
		m_pInstance->RemoveFlashElement();
	}
}

CCreateSinglePlayerGameDialogScaleform::CCreateSinglePlayerGameDialogScaleform(bool bUsingMatchmaking, bool bTeamLobbyMode, bool bTrainingMode) :
    m_bUsingMatchmaking(bUsingMatchmaking),
    m_bTeamLobbyMode(bTeamLobbyMode),
    m_bTrainingMode(bTrainingMode),
    m_iGameType(-1),
    m_iGameMode(-1),
    m_iAnyType(-1)
{

    m_szMatchmakingQuery[0] = 0;
    m_szMapsToLaunch[0] = 0;

	g_pMatchFramework->GetEventsSubscription()->Subscribe(this);
}

CCreateSinglePlayerGameDialogScaleform::~CCreateSinglePlayerGameDialogScaleform()
{
	g_pMatchFramework->GetEventsSubscription()->Unsubscribe(this);
}

void CCreateSinglePlayerGameDialogScaleform::FlashLoaded()
{
	const int iNumInitDialogDataParams = 3;
	WITH_SFVALUEARRAY(data, iNumInitDialogDataParams)
	{
		m_pScaleformUI->ValueArray_SetElement(data, 0, m_bUsingMatchmaking);
		m_pScaleformUI->ValueArray_SetElement(data, 1, m_bTeamLobbyMode);
		m_pScaleformUI->ValueArray_SetElement(data, 2, m_bTrainingMode);

		m_pScaleformUI->Value_InvokeWithoutReturn(m_FlashAPI, "InitDialogData", data, iNumInitDialogDataParams);
	}
}

void CCreateSinglePlayerGameDialogScaleform::FlashReady()
{
	Show();
}

void CCreateSinglePlayerGameDialogScaleform::PostUnloadFlash()
{
    StopListeningForAllEvents();

    g_pScaleformUI->RefreshKeyBindings();

    if (this->m_szMatchmakingQuery[0])
    {
        if (this->m_bTeamLobbyMode && BasePanel()->InTeamLobby())
        {
            if (/*mm_queue.GetInt() &&*/ this->m_iGameType == 0 && this->m_iGameMode == 1 && !V_stristr(this->m_szMapsToLaunch, "@workshop"))
            {
                // player_competitive_maplist->SetValue(this->m_szMapsToLaunch);
                engine->ClientCmd_Unrestricted("host_writeconfig\n");
            }
            const char* szWorkshopMap = V_stristr(this->m_szMapsToLaunch, "@workshop");
            // CLobbyScreen_Scaleform::UpdateCustomMatchSettings(m_iGameType, m_iGameMode, m_szMapsToLaunch, szWorkshopMap == nullptr ? m_iAnyType : 0);
            BasePanel()->RestoreMainMenuScreen();
        }
        else
        {
            KeyValues* pKVSettings = KeyValues::FromString("Settings", this->m_szMatchmakingQuery);
            const char* szGameType = pKVSettings->GetString("game/type");
            const char* szGameMode = pKVSettings->GetString("game/mode");
            const char* szMapGroupName = pKVSettings->GetString("game/mapgroupname");
            int iAnyTypeMode = pKVSettings->GetInt("options/anytypemode");
            int iGamePrime = pKVSettings->GetInt("game/prime");

            BasePanel()->SetSinglePlayer(!this->m_bUsingMatchmaking);
            if (this->m_bUsingMatchmaking)
            {
                int iGameType = -1;
                int iGameMode = -1;
                g_pGameTypes->GetGameModeAndTypeIntsFromStrings(szGameType, szGameMode, iGameType, iGameMode);

                /*if (mm_queue.GetInt())
                {
                    if (iGameType == 0 && iGameMode == 1)
                    {
                        if (!V_stristr(szMapGroupName, "@workshop"))
                        {
                            player_competitive_maplist->SetValue(szMapGroupName);
                            engine->ClientCmd_Unrestricted("host_writeconfig\n");

                            CLobbyScreen_Scaleform::LoadDialog(1, iGameType, iGameMode, iAnyTypeMode, szMapGroupName, 2, 0);
                            if (CLobbyScreen_Scaleform::m_pInstance)
                            {
                                CFmtStrN<256, false>::CFmtStrN(v38, "game { prime %d }", iGamePrime != 0);
                                CLobbyScreen_Scaleform::m_pInstance->SetGameExtrasForLobbyCreate(v39);
                            }
                            m_pInstance = nullptr;
                            delete this;
                            return;
                        }
                    }
                }*/

                if (/*mm_queue.GetInt() < 2 ||*/ V_stristr(szMapGroupName, "@workshop"))
                {
                    g_pGameTypes->SetCustomBotDifficulty(2);

                    pKVSettings->SetString("system/network", "offline");
                    pKVSettings->SetString("system/access", "private");

                    g_pMatchFramework->CreateSession(pKVSettings);
                    IMatchSession* pMatchSession = g_pMatchFramework->GetMatchSession();
                    if (pMatchSession)
                    {
                        KeyValues* pKVStart = new KeyValues("Start");
                        pMatchSession->Command(pKVStart);
                    }
                    else
                    {
                        Warning("CCreateSinglePlayerGameDialogScaleform: unable to create single player session.\n");
                        BasePanel()->RestoreMainMenuScreen();
                    }
                }
                else
                {
                    // CLobbyScreen_Scaleform::LoadDialog(1, iGameType, iGameMode, iAnyTypeMode, szMapGroupName, 2, 0);
                }
            }
            else
            {
                if (!V_stricmp(szGameMode, "competitive"))
                {
                    // CSAppLifetimeGameStats()->RecordUIEvent("SoloSearch");
                }

                g_pMatchFramework->CreateSession(pKVSettings);
                IMatchSession* pMatchSession = g_pMatchFramework->GetMatchSession();
                if (pMatchSession)
                {
                    KeyValues* pKVStart = new KeyValues("Start");
                    pMatchSession->Command(pKVStart);
                }
                else
                {
                    Warning("CCreateSinglePlayerGameDialogScaleform: unable to create single player session.\n");
                    BasePanel()->RestoreMainMenuScreen();
                }
            }
        }
    }
    else
    {
        BasePanel()->RestoreMainMenuScreen();
    }

    m_pInstance = nullptr;
    delete this;
}

void CCreateSinglePlayerGameDialogScaleform::Show()
{
	ListenForGameEvent("mb_input_lock_success");
	ListenForGameEvent("mb_input_lock_cancel");
	ListenForGameEvent("ugc_map_info_received");
	ListenForGameEvent("ugc_map_unsubscribed");
	ListenForGameEvent("ugc_map_download_error");

	WITH_SLOT_LOCKED
	{
		ScaleformUI()->Value_InvokeWithoutReturn(m_FlashAPI, "showPanel", NULL, 0);
	}
}

void CCreateSinglePlayerGameDialogScaleform::Hide()
{
	StopListeningForAllEvents();

	if (m_szMatchmakingQuery && !m_bUsingMatchmaking)
	{
		GameUI().StartBackgroundMusicFade();
	}

	WITH_SLOT_LOCKED
	{
		ScaleformUI()->Value_InvokeWithoutReturn(m_FlashAPI, "hidePanel", NULL, 0);
	}
}

void CCreateSinglePlayerGameDialogScaleform::OnEvent(KeyValues* event)
{
    SF_COMPONENT_FORWARD_EVENT(event->GetName());
}

void CCreateSinglePlayerGameDialogScaleform::FireGameEvent(IGameEvent* event)
{
	const char* name = event->GetName();
	if (!V_strcmp(name, "mb_input_lock_success"))
	{
		Hide();
	}
	else if (!V_strcmp(name, "mb_input_lock_cancel"))
	{
		m_szMatchmakingQuery[0] = 0;
	}
	else if (!V_strcmp(name, "ugc_map_info_received") || !V_strcmp(name, "ugc_map_unsubscribed") || !V_strcmp(name, "ugc_map_download_error"))
	{
		// FilterWorkshopMapsByTagsInternal(this, m_WorkshopMapsTags.Get());
	}
}

SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetQueuedMatchmakingTime) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetQueuedMatchmakingPlayers)
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetQueuedMatchmakingPreferredMaplist) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::FilterWorkshopMapsByTags) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::ViewMapInWorkshop) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::ViewAllMapsInWorkshop) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetWorkshopMapPath) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetWorkshopMapID) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::GetWorkshopMapDownloadProgress) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::EnumerateWorkshopMapsFailed) // used
SFUI_CALLBACK_TODO(CCreateSinglePlayerGameDialogScaleform::RefreshFileInfo)

void CCreateSinglePlayerGameDialogScaleform::UpdatePendingInvites(SCALEFORM_CALLBACK_ARGS_DECL) {}
void CCreateSinglePlayerGameDialogScaleform::ShowInviteOverlay(SCALEFORM_CALLBACK_ARGS_DECL) {}
void CCreateSinglePlayerGameDialogScaleform::DownloadCurrentGamesCount(SCALEFORM_CALLBACK_ARGS_DECL) {}

void CCreateSinglePlayerGameDialogScaleform::UpdatedSelections(SCALEFORM_CALLBACK_ARGS_DECL)
{
    if (m_bTeamLobbyMode && pui->Params_GetNumArgs(obj) >= 2)
    {
        m_iGameType = pui->Params_GetArgAsNumber(obj, 0);
        m_iGameMode = pui->Params_GetArgAsNumber(obj, 1);
        V_strncpy(m_szMapsToLaunch, pui->Params_GetArgAsString(obj, 2), sizeof(m_szMapsToLaunch));
        m_iAnyType = pui->Params_GetArgAsNumber(obj, 3);
    }
}

void CCreateSinglePlayerGameDialogScaleform::OnOk(SCALEFORM_CALLBACK_ARGS_DECL)
{
	engine->ClientCmd_Unrestricted(VarArgs("host_writeconfig_ss %d", XBX_GetActiveUserId()));
	if (!BasePanel()->ShowLockInput())
	{
		Hide();
	}
}

void CCreateSinglePlayerGameDialogScaleform::CheckGameSettingsRequirements(SCALEFORM_CALLBACK_ARGS_DECL)
{
	pui->Params_GetArgAsString(obj, 0);
	pui->Params_SetResult(obj, 1);
}

void CCreateSinglePlayerGameDialogScaleform::SetMatchmakingQuery(SCALEFORM_CALLBACK_ARGS_DECL)
{
	const char* szQuery = pui->Params_GetArgAsString(obj, 0);
	V_strncpy(m_szMatchmakingQuery, szQuery, sizeof(m_szMatchmakingQuery));
}

void CCreateSinglePlayerGameDialogScaleform::SetCustomBotDifficulty(SCALEFORM_CALLBACK_ARGS_DECL)
{
	int iDifficulty = pui->Params_GetArgAsNumber(obj, 0);
	g_pGameTypes->SetCustomBotDifficulty(iDifficulty);
}

#endif // INCLUDE_SCALEFORM