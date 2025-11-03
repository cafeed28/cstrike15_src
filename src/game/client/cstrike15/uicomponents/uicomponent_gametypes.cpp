#include "cbase.h"
#include "uicomponent_gametypes.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

SF_COMPONENT_API_DEF_BEGIN(CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetGameTypeAttribute, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetGameModeType, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetGameModeAttribute, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetMapGroupsCountByGameTypeModePlay, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetMapGroupByGameTypeModePlayIndex, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetMapGroupAttribute, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetMapGroupAttributeSubKeys, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetMapsCountByMapGroup, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetMapByMapGroupAndIndex, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetMapAttribute, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(int, GetActiveSeasionIndexValue, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetActiveSeasionCodeName, CUiComponent_GameTypes)
	SF_COMPONENT_FUNCTION_API_DEF(const char*, GetGameModesJSON, CUiComponent_GameTypes)
SF_COMPONENT_API_DEF_END(CUiComponent_GameTypes)

UI_COMPONENT_API_DEF_COMMON(CUiComponent_GameTypes, GameTypes)

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetGameTypeAttribute)
{
	const char* szResult = "";

	const char* szGameType = pui->Params_GetArgAsString(obj, 0);
	const char* szAttribute = pui->Params_GetArgAsString(obj, 1);

	if (szGameType && *szGameType && szAttribute && *szAttribute)
	{
		szResult = m_pKV->FindKey("gameTypes")->FindKey(szGameType)->GetString(szAttribute);
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetGameModeType)
{
	const char* szResult = GetGameModeType(pui->Params_GetArgAsString(obj, 0));

	if (szResult)
	{
		pui->Params_SetResult(obj, szResult);
	}
	else
	{
		Assert(false);
	}
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetGameModeAttribute)
{
	const char* szGameType = pui->Params_GetArgAsString(obj, 0);
	const char* szGameMode = pui->Params_GetArgAsString(obj, 1);
	const char* szAttribute = pui->Params_GetArgAsString(obj, 2);

	const char* szResult = GetGameModeAttribute(szGameType, szGameMode, szAttribute);

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapGroupsCountByGameTypeModePlay)
{
	int nResult = 0;

	const char* szGameType = pui->Params_GetArgAsString(obj, 0);
	const char* szGameMode = pui->Params_GetArgAsString(obj, 1);
	const char* szMapGroupMode = pui->Params_GetArgAsString(obj, 2);

	if (szGameType && *szGameType && szGameMode && *szGameMode)
	{
		if (szMapGroupMode && V_stricmp(szMapGroupMode, "SP"))
		{
			szMapGroupMode = "mapgroupsMP";
		}
		else
		{
			szMapGroupMode = "mapgroupsSP";
		}

		FOR_EACH_SUBKEY(m_pKV->FindKey("gameTypes")->FindKey(szGameType)->FindKey("gameModes")->FindKey(szGameMode)->FindKey(szMapGroupMode), i)
			++nResult;
	}

	pui->Params_SetResult(obj, nResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapGroupByGameTypeModePlayIndex)
{
	const char* szResult = "";

	const char* szGameType = pui->Params_GetArgAsString(obj, 0);
	const char* szGameMode = pui->Params_GetArgAsString(obj, 1);
	const char* szMapGroupMode = pui->Params_GetArgAsString(obj, 2);
	int iIndex = pui->Params_GetArgAsNumber(obj, 3) - 1;

	if (szGameType && *szGameType && szGameMode && *szGameMode)
	{
		if (szMapGroupMode && V_stricmp(szMapGroupMode, "SP"))
		{
			szMapGroupMode = "mapgroupsMP";
		}
		else
		{
			szMapGroupMode = "mapgroupsSP";
		}

		if (iIndex >= 0)
		{
			FOR_EACH_SUBKEY(m_pKV->FindKey("gameTypes")->FindKey(szGameType)->FindKey("gameModes")->FindKey(szGameMode)->FindKey(szMapGroupMode), i)
			{
				if (iIndex-- <= 0)
				{
					szResult = i->GetName();
					break;
				}
			}
		}
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapGroupAttribute)
{
	const char* szMapGroup = pui->Params_GetArgAsString(obj, 0);
	const char* szAttribute = pui->Params_GetArgAsString(obj, 1);

	const char* szResult = "";

	if (szMapGroup && *szMapGroup && szAttribute && *szAttribute)
	{
		szResult = m_pKV->FindKey("mapgroups")->FindKey(szMapGroup)->GetString(szAttribute);
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapGroupAttributeSubKeys)
{
	const char* szMapGroup = pui->Params_GetArgAsString(obj, 0);
	const char* szAttribute = pui->Params_GetArgAsString(obj, 1);

	const char* szResult = GetMapGroupAttributeSubKeys(szMapGroup, szAttribute);

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapsCountByMapGroup)
{
	int nResult = 0;

	const char* szMapGroup = pui->Params_GetArgAsString(obj, 0);

	if (szMapGroup && *szMapGroup)
	{
		FOR_EACH_SUBKEY(m_pKV->FindKey("mapgroups")->FindKey(szMapGroup)->FindKey("maps"), i)
			++nResult;
	}

	pui->Params_SetResult(obj, nResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapByMapGroupAndIndex)
{
	const char* szResult = "";

	const char* szMapGroup = pui->Params_GetArgAsString(obj, 0);
	int iIndex = pui->Params_GetArgAsNumber(obj, 1) - 1;

	if (szMapGroup && *szMapGroup)
	{
		if (iIndex >= 0)
		{
			FOR_EACH_SUBKEY(m_pKV->FindKey("mapgroups")->FindKey(szMapGroup)->FindKey("maps"), i)
			{
				if (iIndex-- <= 0)
				{
					szResult = i->GetName();
					break;
				}
			}
		}
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetMapAttribute)
{
	const char* szMap = pui->Params_GetArgAsString(obj, 0);
	const char* szAttribute = pui->Params_GetArgAsString(obj, 1);

	const char* szResult = "";

	if (szMap && *szMap && szAttribute && *szAttribute)
	{
		szResult = m_pKV->FindKey("maps")->FindKey(szMap)->GetString(szAttribute);
	}

	pui->Params_SetResult(obj, szResult);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetActiveSeasionIndexValue)
{
	pui->Params_SetResult(obj, -1);
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetActiveSeasionCodeName)
{
	pui->Params_SetResult(obj, "");
}

SF_COMPONENT_FUNCTION_IMPL(CUiComponent_GameTypes, GetGameModesJSON)
{
	pui->Params_SetResult(obj, (const char*)m_buf.Base());
}

const char* CUiComponent_GameTypes::GetGameTypeAttribute(const char* szGameType, const char* szGameMode, const char* szAttribute)
{
	if (!szGameType || !*szGameType || !szGameMode || !*szGameMode || !szAttribute || !*szAttribute)
		return "";

	return m_pKV->FindKey("gameTypes")->FindKey(szGameType)->FindKey("gameModes")->FindKey(szGameMode)->GetString(szAttribute);
}

const char* CUiComponent_GameTypes::GetGameModeType(const char* szGameMode)
{
	FOR_EACH_SUBKEY(m_pKV->FindKey("gameTypes"), i)
	{
		FOR_EACH_SUBKEY(i->FindKey("gameModes"), j)
		{
			if (!V_stricmp(j->GetName(), szGameMode) && j->FindKey("value") && j->FindKey("maxplayers"))
				return i->GetName();
		}
	}

	return NULL;
}

const char* CUiComponent_GameTypes::GetGameModeAttribute(const char* szGameType, const char* szGameMode, const char* szAttribute)
{
	if (!szGameType || !*szGameType || !szGameMode || !*szGameMode || !szAttribute || !*szAttribute)
		return "";

	return m_pKV->FindKey("gameTypes")->FindKey(szGameType)->FindKey("gameModes")->FindKey(szGameMode)->GetString(szAttribute);
}

const char* CUiComponent_GameTypes::GetMapGroupAttributeSubKeys(const char* szMapGroup, const char* szAttribute)
{
	static CFmtStr1024 s_fmtResult;
	s_fmtResult.Clear();

	if (!(!szMapGroup || !*szMapGroup || !szAttribute || !*szAttribute))
	{
		FOR_EACH_SUBKEY(m_pKV->FindKey("mapgroups")->FindKey(szMapGroup)->FindKey(szAttribute), i)
		{
			if (s_fmtResult.Length() > 0) s_fmtResult.Append(',');
			s_fmtResult.AppendFormat("%s", i->GetName());
		}
	}

	return s_fmtResult.Get();
}

CUiComponent_GameTypes::CUiComponent_GameTypes()
: m_buf()
{
	m_pKV = new KeyValues("GameTypes");
	if (!m_pKV->LoadFromFile(g_pFullFileSystem, "GameModes.txt", 0, 0))
	{
		m_pKV->deleteThis();
		m_pKV = NULL;
	}
	// RE TODO: GameTypesRecursivelyCensorKeyValues(m_pKV);
	Helper_RecursiveKeyValuesToJSONString(m_pKV, m_buf);
}

CUiComponent_GameTypes::~CUiComponent_GameTypes()
{
	if (m_pKV)
	{
		m_pKV->deleteThis();
		m_pKV = NULL;
	}
}