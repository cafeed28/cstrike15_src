#pragma once

#include "uicomponent_common.h"
#include "matchmaking/imatchframework.h"

class CUiComponent_GameTypes : public CUiComponentGlobalInstanceHelper<CUiComponent_GameTypes> {
	UI_COMPONENT_DECLARE_GLOBAL_INSTANCE_ONLY(CUiComponent_GameTypes);

public:
	SF_COMPONENT_FUNCTION(const char*, GetGameTypeAttribute);
	SF_COMPONENT_FUNCTION(const char*, GetGameModeType);
	SF_COMPONENT_FUNCTION(const char*, GetGameModeAttribute);
	SF_COMPONENT_FUNCTION(int, GetMapGroupsCountByGameTypeModePlay);
	SF_COMPONENT_FUNCTION(const char*, GetMapGroupByGameTypeModePlayIndex);
	SF_COMPONENT_FUNCTION(const char*, GetMapGroupAttribute);
	SF_COMPONENT_FUNCTION(const char*, GetMapGroupAttributeSubKeys);
	SF_COMPONENT_FUNCTION(int, GetMapsCountByMapGroup);
	SF_COMPONENT_FUNCTION(const char*, GetMapByMapGroupAndIndex);
	SF_COMPONENT_FUNCTION(const char*, GetMapAttribute);
	SF_COMPONENT_FUNCTION(int, GetActiveSeasionIndexValue);
	SF_COMPONENT_FUNCTION(const char*, GetActiveSeasionCodeName);
	SF_COMPONENT_FUNCTION(const char*, GetGameModesJSON);

	const char* GetGameTypeAttribute(const char* szParam0, const char* szParam1, const char* szParam2);
	const char* GetGameModeType(const char* szParam0);
	const char* GetGameModeAttribute(const char* szParam0, const char* szParam1, const char* szParam2);
	const char* GetMapGroupAttributeSubKeys(const char* szParam0, const char* szParam1);

private:
	KeyValues* m_pKV;
	CUtlBuffer m_buf;
};