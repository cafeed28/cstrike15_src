#if defined( INCLUDE_SCALEFORM )
#pragma once

#include "scaleformui/scaleformui.h"
#include "matchmaking/imatchframework.h"
#include "uigamedata.h"
#include "gameui_interface.h"
#include "gameeventlistener.h"

class CPauseMenuScreenScaleform : public ScaleformFlashInterfaceMixin<CGameEventListener>, public IMatchEventsSink
{
protected:
	static CPauseMenuScreenScaleform* m_pInstance;

	CPauseMenuScreenScaleform();
	~CPauseMenuScreenScaleform();

public:
	static void LoadDialog();
	static void UnloadDialog();
	static void RestorePanel();
	static void ShowMenu(bool bShow);
	static bool IsActive() { return m_pInstance != NULL; }
	static CPauseMenuScreenScaleform* GetInstance() { return m_pInstance; }

	void Show();
	void Hide();

	SF_COMPONENT_HOST_DECL();
	void BasePanelRunCommand(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsQueuedMatchmaking(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsGotvSpectating(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsMultiplayer(SCALEFORM_CALLBACK_ARGS_DECL);
	void NeedsInviteFriends(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsTraining(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetTeamNumber(SCALEFORM_CALLBACK_ARGS_DECL);
	void CanMakeSessionPublic(SCALEFORM_CALLBACK_ARGS_DECL);
	void SwitchTeams(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsPlayingGunGameProgressive(SCALEFORM_CALLBACK_ARGS_DECL);
	void CallVote(SCALEFORM_CALLBACK_ARGS_DECL);
	void OpenPlayerDetailsPanel(SCALEFORM_CALLBACK_ARGS_DECL);
	void IsWorkshopMap(SCALEFORM_CALLBACK_ARGS_DECL);
	void ViewMapInWorkshop(SCALEFORM_CALLBACK_ARGS_DECL);

	// IGameEventListener2
	void FireGameEvent(IGameEvent* event) override;

	// IMatchEventsSink
	void OnEvent(KeyValues* pEvent) override;

protected:
	void DestroyDialog();
	void PrepareUIForPauseMenu(bool bUnk);
	void InnerRestorePanel();

	void FlashLoaded() override;
	void FlashReady() override;
	void PostUnloadFlash() override;

protected:
	int m_iSplitScreenSlot;
	bool m_bVisible;
	bool m_bOwningInput;
	bool m_bUnk;
	float m_flTimeScale;
};

#endif // INCLUDE_SCALEFORM
