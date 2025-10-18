#pragma once

#include "uicomponent_common.h"

class CUiComponent_MyPersona : public CUiComponentGlobalInstanceHelper<CUiComponent_MyPersona> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_MyPersona);

public:
	SF_COMPONENT_FUNCTION(uint64, GetXuid);
	SF_COMPONENT_FUNCTION(const wchar_t*, GetName);
	SF_COMPONENT_FUNCTION(bool, IsVacBanned);

private:
	static CSteamID s_steamIdMyself;

public:
	void RequestAccountPrivacySettings();
};