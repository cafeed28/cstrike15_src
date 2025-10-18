#if defined( INCLUDE_SCALEFORM )
#pragma once

#include "scaleformui/scaleformui.h"
#include "matchmaking/imatchframework.h"
#include "gameeventlistener.h"
#include "gametypes.h"

class CCreateSinglePlayerGameDialogScaleform : public ScaleformFlashInterfaceMixin<CGameEventListener>, public IMatchEventsSink
{
protected:
	static CCreateSinglePlayerGameDialogScaleform* m_pInstance;

	CCreateSinglePlayerGameDialogScaleform(bool bUsingMatchmaking, bool bTeamLobbyMode, bool bTrainingMode);
	~CCreateSinglePlayerGameDialogScaleform();

public:
	static void LoadDialog(bool bUsingMatchmaking, bool bTeamLobbyMode, bool bTrainingMode);
	static void UnloadDialog();

	void Show();
	void Hide();
	
	void OnOk(SCALEFORM_CALLBACK_ARGS_DECL);
	void CheckGameSettingsRequirements(SCALEFORM_CALLBACK_ARGS_DECL);
	void SetMatchmakingQuery(SCALEFORM_CALLBACK_ARGS_DECL);
	void SetCustomBotDifficulty(SCALEFORM_CALLBACK_ARGS_DECL);
	void UpdatedSelections(SCALEFORM_CALLBACK_ARGS_DECL);
	void UpdatePendingInvites(SCALEFORM_CALLBACK_ARGS_DECL);
	void ShowInviteOverlay(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetQueuedMatchmakingTime(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetQueuedMatchmakingPlayers(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetQueuedMatchmakingPreferredMaplist(SCALEFORM_CALLBACK_ARGS_DECL);
	void FilterWorkshopMapsByTags(SCALEFORM_CALLBACK_ARGS_DECL);
	void ViewMapInWorkshop(SCALEFORM_CALLBACK_ARGS_DECL);
	void ViewAllMapsInWorkshop(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetWorkshopMapPath(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetWorkshopMapID(SCALEFORM_CALLBACK_ARGS_DECL);
	void GetWorkshopMapDownloadProgress(SCALEFORM_CALLBACK_ARGS_DECL);
	void EnumerateWorkshopMapsFailed(SCALEFORM_CALLBACK_ARGS_DECL);
	void RefreshFileInfo(SCALEFORM_CALLBACK_ARGS_DECL);
	void DownloadCurrentGamesCount(SCALEFORM_CALLBACK_ARGS_DECL);

	// CGameEventListener
	void FireGameEvent(IGameEvent* event) override;

	// IMatchEventsSink
	void OnEvent(KeyValues* event) override;

protected:
	void FlashLoaded() override;
	void FlashReady() override;
	void PostUnloadFlash() override;

	// CFmtStr<256> m_fmtUnk;
	// CUtlVector<char*> m_vUnk;
	CCallResult<CCreateSinglePlayerGameDialogScaleform, LobbyMatchList_t> m_CallResultLobbyMatchList;
	// void* m_pUnk;

	char m_szMatchmakingQuery[8192];
	bool m_bUsingMatchmaking;
	bool m_bTeamLobbyMode;
	bool m_bTrainingMode;

	int m_iGameType;
	int m_iGameMode;
	int m_iAnyType;
	char m_szMapsToLaunch[4096]; // or maptolaunch

	// CUtlVector<PublishedFileInfo_t> m_vPublishedFileData;
	// CUtlString m_WorkshopMapsTags;
};

#endif // INCLUDE_SCALEFORM