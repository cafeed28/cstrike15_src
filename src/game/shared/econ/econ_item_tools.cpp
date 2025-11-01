//========= Copyright Valve Corporation, All rights reserved. ============//
#include "cbase.h"
#include "game_item_schema.h"
#include "econ_item_interface.h"
#include "econ_item_tools.h"
#include "econ_item_constants.h"
#include "schemainitutils.h"
#include "rtime.h"



bool BStringsEqual( const char *pszA, const char *pszB )
{
	if ( pszA == NULL )
		return pszB == NULL;

	if ( pszB == NULL )
		return false;

	return !V_strcmp( pszA, pszB );
}

const unsigned int g_CapabilityApplicationMap[] =
{
															// if we have a tool that has this capability...
	// ...then we check to see if the tool target has
	// this capability

	ITEM_CAP_PAINTABLE,										// ITEM_CAP_PAINTABLE
	ITEM_CAP_NAMEABLE,										// ITEM_CAP_NAMEABLE
	ITEM_CAP_DECODABLE,										// ITEM_CAP_DECODABLE
	0,														// ITEM_CAP_UNUSED; was ITEM_CAP_CAN_MOD_SOCKET
	ITEM_CAP_CAN_CUSTOMIZE_TEXTURE,							// ITEM_CAP_CAN_CUSTOMIZE_TEXTURE
	0,														// ITEM_CAP_USABLE
	0,														// ITEM_CAP_USABLE_GC
	ITEM_CAP_CAN_GIFT_WRAP,									// ITEM_CAP_CAN_GIFT_WRAP
	0,														// ITEM_CAP_USABLE_OUT_OF_GAME
	ITEM_CAP_CAN_COLLECT,									// ITEM_CAP_CAN_COLLECT
	0,														// ITEM_CAP_CAN_CRAFT_COUNT
	0,														// ITEM_CAP_CAN_CRAFT_MARK
	ITEM_CAP_PAINTABLE_TEAM_COLORS | ITEM_CAP_PAINTABLE,	// ITEM_CAP_PAINTABLE_TEAM_COLORS
	0,														// ITEM_CAP_CAN_BE_RESTORED
	ITEM_CAP_CAN_USE_STRANGE_PARTS,							// ITEM_CAP_CAN_USE_STRANGE_PARTS
	ITEM_CAP_PAINTABLE_UNUSUAL,								// ?
	ITEM_CAP_CAN_INCREMENT,									// ?
	ITEM_CAP_USES_ESSENCE,									// ?
	ITEM_CAP_AUTOGRAPH,										// ?
	ITEM_CAP_RECIPE,										// ?
	ITEM_CAP_CAN_STICKER,									// ?
	ITEM_CAP_STATTRACK_SWAP,								// ?
};

COMPILE_TIME_ASSERT( ARRAYSIZE( g_CapabilityApplicationMap ) == NUM_ITEM_CAPS );

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
bool IEconTool::ShouldDisplayQuantity( const IEconItemInterface *pTool ) const
{
	Assert( pTool );

	const GameItemDefinition_t *pItemDef = pTool->GetItemDefinition();
	if ( !pItemDef )
		return false;

	static CSchemaAttributeDefHandle pAttrDef_UnlimitedQuantity( "unlimited quantity" );
	if ( pTool->FindAttribute( pAttrDef_UnlimitedQuantity ) )
		return false;

	if ( pTool->GetQuantity() >= 0 )
	{
		if ( (pItemDef->GetCapabilities() & ITEM_CAP_USABLE_GC) != 0 )
			return true;

		if ( pItemDef->IsTool() )
			return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
CEconTool_WrappedGift::CEconTool_WrappedGift( const char *pszTypeName, const char *pszUseString, item_capabilities_t unCapabilities, KeyValues *pUsageKV )
	: IEconTool( pszTypeName, pszUseString, NULL, unCapabilities, pUsageKV )
	, m_pszDeliveredGiftItemDefName( NULL )
	, m_pDeliveredGiftItemDef( NULL )
	, m_bIsGlobalGift( false )
	, m_bIsDirectGift( false )
{
	if ( pUsageKV )
	{
		m_bIsGlobalGift = pUsageKV->GetBool( "target_type_global", false );
		m_pszDeliveredGiftItemDefName = pUsageKV->GetString( "delivered_gift_item_def", NULL );
		m_bIsDirectGift = pUsageKV->GetBool( "is_direct_gift", false );
	}
}

/*
//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
CEconTool_GiftWrap::CEconTool_GiftWrap( const char *pszTypeName, const char *pszUseString, item_capabilities_t unCapabilities, KeyValues *pUsageKV )
	: IEconTool( pszTypeName, NULL, NULL, unCapabilities )
	, m_pszWrappedGiftItemDefName( NULL )
	, m_pWrappedGiftItemDef( NULL )
{
	if ( pUsageKV )
	{
		m_pszWrappedGiftItemDefName = pUsageKV->GetString( "wrapped_gift_item_def", NULL );
	}
}

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
bool CEconTool_GiftWrap::BFinishInitialization()
{
	// Now that we've finished parsing our definitions, look for a match.
	if ( m_pszWrappedGiftItemDefName )
	{
		m_pWrappedGiftItemDef = GetItemSchema()->GetItemDefinitionByName( m_pszWrappedGiftItemDefName );
	}

	// We're done with this value.
	m_pszWrappedGiftItemDefName = NULL;

	return m_pWrappedGiftItemDef != NULL
		&& IEconTool::BFinishInitialization();
}

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
bool CEconTool_GiftWrap::CanApplyTo( const IEconItemInterface *pTool, const IEconItemInterface *pToolSubject ) const
{
	Assert( pTool );
	Assert( pToolSubject );

	if ( pToolSubject->GetQuality() == AE_SELFMADE ||
		 pToolSubject->GetQuality() == AE_COMMUNITY ||
		 pToolSubject->GetQuality() == AE_CUSTOMIZED ||
		 pToolSubject->GetQuality() == AE_NORMAL )
	{
		return false;
	}

	// If an item is currently trade restricted, other flags don't matter (I think).
	if ( ( pToolSubject->GetUntradabilityFlags() & k_Untradability_Temporary ) != 0 )
		return false;

	// One item still has the gift wrap cap, which is the engagement ring ( Something Special For Someone Special (Tool) ).
	// However, the can_gift_wrap cap shouldn't overcome temporary trade restrictions.
	static CSchemaAttributeDefHandle pAttrDef_ToolNeedsGiftwrap( "tool needs giftwrap" );

	const bool cbSubjectNeedsGiftWrap = pToolSubject->FindAttribute( pAttrDef_ToolNeedsGiftwrap );
	const bool cbSubjectCanTrade = pToolSubject->IsTradable();
	const bool cbSubjectCanProceed = cbSubjectNeedsGiftWrap || cbSubjectCanTrade;
 
	if ( !cbSubjectCanProceed )
		return false;

	static CSchemaAttributeDefHandle pAttrDef_CannotGiftwrap( "cannot giftwrap" );
	if ( pToolSubject->FindAttribute( pAttrDef_CannotGiftwrap ) )
		return false;

	return IEconTool::CanApplyTo( pTool, pToolSubject );
}
*/

//---------------------------------------------------------------------------------------
// Purpose: given a tool and an item to apply the tool's effects upon, return true if the
//			tool is allowed to affect the subject. This is used on the client for UI and
//			on the GC for actual application validity testing.
//---------------------------------------------------------------------------------------
/* static */ bool CEconSharedToolSupport::ToolCanApplyTo( const IEconItemInterface *pTool, const IEconItemInterface *pToolSubject, item_capabilities_t unCapabilities )
{
	if ( pTool == NULL || pToolSubject == NULL )
		return false;

	const GameItemDefinition_t *pToolDef = pTool->GetItemDefinition();
	if ( pToolDef == NULL )
		return false;

	// If we have a tool that's in escrow it can't be used on anything.
	static CSchemaAttributeDefHandle pAttrib_ToolEscrowUntil( "tool escrow until date" );
	if ( pTool->FindAttribute( pAttrib_ToolEscrowUntil ) )
		return false;
	
	static CSchemaAttributeDefHandle pAttrib_UseAfterDate( "use after date" );
	uint32 unUseAfterDate;
	if ( pTool->FindAttribute( pAttrib_UseAfterDate, &unUseAfterDate ) && unUseAfterDate > CRTime::RTime32TimeCur() )
		return false;

	// Cannot modify preview items. Should be caught by temporary-item check above.
	Assert( pToolSubject->GetOrigin() != kEconItemOrigin_PreviewItem );

	const GameItemDefinition_t *pToolSubjectDef = pToolSubject->GetItemDefinition();
	if ( pToolSubjectDef == NULL )
		return false;

	if ( !ToolCanApplyToDefinition( pToolDef, pToolSubjectDef, unCapabilities ) )
		return false;

	// If we can apply to the definition then we should be known to have valid tool data.
	// If our tool has no tool metadata then we don't allow it to be applied to anything.
	const IEconTool *pEconTool = pToolDef->GetEconTool();
	Assert( pEconTool );

	return pEconTool->CanApplyTo( pTool, pToolSubject );
}

/* static */ bool CEconSharedToolSupport::ToolCanApplyToDefinition( const GameItemDefinition_t *pToolDef, const GameItemDefinition_t *pToolSubjectDef, item_capabilities_t unCapabilities )
{
	if ( !pToolDef || !pToolSubjectDef || !pToolDef->IsTool() )
	{
		// not a tool
		return false;
	}

	// If our tool has no tool metadata then we don't allow it to be applied to anything.
	const IEconTool *pEconTool = pToolDef->GetEconTool();
	if ( !pEconTool )
		return false;

	unsigned int unToolUsageCaps = 0;
	if ( pEconTool->GetCapabilities() )
	{
		for ( unsigned int i = 0; i < NUM_ITEM_CAPS; i++ )
		{
			if ( pEconTool->GetCapabilities() & (1 << i) )
			{
				unToolUsageCaps |= g_CapabilityApplicationMap[i];
			}
		}
	}
	
	// Check for base applicability of this tool to this object.
	if ( (unToolUsageCaps & pToolSubjectDef->GetCapabilities()) == 0 )
		return false;

	// check to see if either the tool or the tool target have usage restriction
	const IEconTool *pSubjectEconTool = pToolSubjectDef->GetEconTool();

	if ( pSubjectEconTool )
	{
		// If this tool can apply to anything then we don't care about the checks below
		// making sure restrictions match.
		const char *pszToolRestriction = BStringsEqual( pEconTool->GetUsageRestriction(), "any" )
									   ? pSubjectEconTool->GetUsageRestriction()
									   : pEconTool->GetUsageRestriction();

		if ( !BStringsEqual( pszToolRestriction, pSubjectEconTool->GetUsageRestriction() ) )
			return false;
	}

	return true;
}
