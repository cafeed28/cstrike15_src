//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef ECONITEMTOOLS_H
#define ECONITEMTOOLS_H
#ifdef _WIN32
#pragma once
#endif

enum EConsumptionAttemptResult
{
	kConsumptionResult_CannotConsume,			// could be bum definitions or doesnt meet criteria or anything -- this is failure
	kConsumptionResult_CanConsume,				// able to consume
	kConsumptionResult_WillCompleteCollection,	// able to consume and is the final item to be consumed
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconSharedToolSupport
{
public:
	// Can the given tool instance apply to a specific instance of an item. This should be used in the general
	// case whenever a CEconItem or a CEconItemView is available.
	static bool ToolCanApplyTo( const IEconItemInterface *pToolDef, const IEconItemInterface *pToolSubject, item_capabilities_t unCapabilities );

	// Can the given tool definition apply to an item definition? This will check things like restrictions,
	// matching tool capabilities, etc. but will ignore instance-specific properties. This should only be used
	// by code that doesn't have any access to an instance of the definition.
	static bool ToolCanApplyToDefinition( const GameItemDefinition_t *pToolDef, const GameItemDefinition_t *pToolSubjectDef, item_capabilities_t unCapabilities );

	//static bool CanCollect( const IEconItemInterface *pToolDef, const IEconItemInterface *pToolSubject, unsigned int* pUnk );
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_DuelingMinigame : public IEconTool
{
public:
	CEconTool_DuelingMinigame( const char *pszTypeName, const char *pszUseString ) : IEconTool( pszTypeName, pszUseString, NULL, ITEM_CAP_NONE, NULL ) { }

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL

#ifdef GC_DLL
	virtual class CGCEconConsumableBehavior *CreateGCConsumableBehavior() const;
#endif // GC_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_Noisemaker : public IEconTool
{
public:
	CEconTool_Noisemaker( const char *pszTypeName, const char *pszUseString ) : IEconTool( pszTypeName, pszUseString, NULL, ITEM_CAP_NONE, NULL ) { }

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_WrappedGift : public IEconTool
{
public:
	CEconTool_WrappedGift( const char *pszTypeName, const char *pszUseString, item_capabilities_t unCapabilities, KeyValues *pUsageKV );

	bool BIsGlobalGift() const { return m_bIsGlobalGift; }
	// Allows the item to be directly used rather than via the trading system
	bool BIsDirectGift() const { return m_bIsDirectGift; }
	const CEconItemDefinition *GetDeliveredItemDefinition() const { return m_pDeliveredGiftItemDef; }	// can return NULL! (means "don't change definitions on delivery")

#ifdef CLIENT_DLL
	virtual bool CanBeUsedNow( const IEconItemInterface *pItem ) const;
	virtual bool ShouldShowContainedItemPanel( const IEconItemInterface *pItem ) const;
	virtual const char *GetUseCommandLocalizationToken( const IEconItemInterface *pItem, int i = 0 ) const;
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
	virtual int GetUseCommandCount( const IEconItemInterface *pItem ) const;
	virtual const char* GetUseCommand( const IEconItemInterface *pItem, int i = 0 ) const;
#endif // CLIENT_DLL

#ifdef GC_DLL
	virtual class CGCEconConsumableBehavior *CreateGCConsumableBehavior() const;
#endif // GC_DLL

private:
	const char *m_pszDeliveredGiftItemDefName;			// points to memory inside our init KV -- only valid between the constructor call and the BFinishInitialization() call (this is messy but Fletcher and I agree it makes more sense than switching to a full two-pass schema parse just for this)

	const CEconItemDefinition *m_pDeliveredGiftItemDef;
	bool m_bIsGlobalGift;
	bool m_bIsDirectGift;
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_WeddingRing : public IEconTool
{
public:
	CEconTool_WeddingRing( const char *pszTypeName, const char *pszUseString, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, pszUseString, NULL, unCapabilities, NULL ) { }

	virtual bool RequiresToolEscrowPeriod() const { return false; }

#ifdef CLIENT_DLL
	virtual const char *GetUseCommandLocalizationToken( const IEconItemInterface *pItem, int i = 0 ) const;
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL

#ifdef GC_DLL
	virtual class CGCEconConsumableBehavior *CreateGCConsumableBehavior() const;
#endif // GC_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_BackpackExpander : public IEconTool
{
public:
	CEconTool_BackpackExpander ( const char *pszTypeName, const char *pszUseString, KeyValues *pUsageKV )
		: IEconTool( pszTypeName, pszUseString, NULL, ITEM_CAP_NONE, pUsageKV )
		, m_iBackpackSlots( 0 )
	{
		if ( pUsageKV )
		{
			m_iBackpackSlots = pUsageKV->GetInt( "backpack_slots", 0 );
		}
	}

	int GetBackpackSlots() const { return m_iBackpackSlots; }

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL

#ifdef GC_DLL
	virtual class CGCEconConsumableBehavior *CreateGCConsumableBehavior() const;
#endif // GC_DLL

private:
	int m_iBackpackSlots;
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
enum EGiftTargetRule
{
	kGiftTargetRule_OnlyOthers = 0,
	kGiftTargetRule_OnlySelf = 1,
};

class CEconTool_Gift : public IEconTool
{
public:
	CEconTool_Gift ( const char *pszTypeName, const char *pszUseString, KeyValues *pUsageKV )
		: IEconTool( pszTypeName, pszUseString, NULL, ITEM_CAP_NONE, pUsageKV )
		, m_pszLootListName( NULL )
		, m_iMaxRecipients( 0 )
		, m_eTargetRule( kGiftTargetRule_OnlySelf )
	{
		if ( pUsageKV )
		{
			m_pszLootListName = pUsageKV->GetString( "loot_list", NULL );
			m_iMaxRecipients = pUsageKV->GetInt( "max_recipients", 0 );
			m_eTargetRule = !Q_stricmp( pUsageKV->GetString( "target_rule", "only_others" ), "only_self" )
						  ? kGiftTargetRule_OnlySelf
						  : kGiftTargetRule_OnlyOthers;
		}
	}

	const char	   *GetLootListName() const		{ return m_pszLootListName; }
	int				GetMaxRecipients() const	{ return m_iMaxRecipients; }
	EGiftTargetRule GetTargetRule() const		{ return m_eTargetRule; }

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL

#ifdef GC_DLL
	virtual class CGCEconConsumableBehavior *CreateGCConsumableBehavior() const;
#endif // GC_DLL

private:
	const char *m_pszLootListName;
	int m_iMaxRecipients;
	EGiftTargetRule m_eTargetRule;
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_PaintCan : public IEconTool
{
public:
	CEconTool_PaintCan( const char *pszTypeName, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, NULL, NULL, unCapabilities, NULL ) { }

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
/*class CEconTool_GiftWrap : public IEconTool
{
public:
	CEconTool_GiftWrap( const char *pszTypeName, const char *pszUseString, item_capabilities_t unCapabilities, KeyValues *pUsageKV );

	virtual bool CanApplyTo( const IEconItemInterface *pTool, const IEconItemInterface *pToolSubject ) const;
	virtual bool RequiresToolEscrowPeriod() const { return false; }

	const CEconItemDefinition *GetWrappedItemDefinition() const { Assert( m_pWrappedGiftItemDef ); return m_pWrappedGiftItemDef; }

#ifdef CLIENT_DLL
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL

private:
	const char *m_pszWrappedGiftItemDefName;			// points to memory inside our init KV -- only valid between the constructor call and the BFinishInitialization() call (this is messy but Fletcher and I agree it makes more sense than switching to a full two-pass schema parse just for this)
	const CEconItemDefinition *m_pWrappedGiftItemDef;
};*/

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_NameTag : public IEconTool
{
public:
	CEconTool_NameTag( const char *pszTypeName, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, NULL, NULL, unCapabilities, NULL ) { }

#ifdef CLIENT_DLL
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_DescTag : public IEconTool
{
public:
	CEconTool_DescTag( const char *pszTypeName, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, NULL, NULL, unCapabilities, NULL ) { }

#ifdef CLIENT_DLL
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_CustomizeTexture : public IEconTool
{
public:
	CEconTool_CustomizeTexture( const char *pszTypeName, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, NULL, NULL, unCapabilities, NULL ) { }

#ifdef CLIENT_DLL
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_CrateKey : public IEconTool
{
public:
	CEconTool_CrateKey( const char *pszTypeName, const char *pszUsageRestriction, item_capabilities_t unCapabilities ) : IEconTool( pszTypeName, NULL, pszUsageRestriction, unCapabilities, NULL ) { }

#ifdef CLIENT_DLL
	// virtual void OnClientApplyTool( CEconItemView *pTool, CEconItemView *pSubject, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

//---------------------------------------------------------------------------------------
// Purpose:
//---------------------------------------------------------------------------------------
class CEconTool_Default : public IEconTool
{
public:
	CEconTool_Default( const char *pszTypeName, const char *pszUseString, const char *pszUsageRestriction, item_capabilities_t unCapabilities )
		: IEconTool( pszTypeName, pszUseString, pszUsageRestriction, unCapabilities, NULL )
	{
		Assert( pszTypeName );
	}

#ifdef CLIENT_DLL
	virtual void OnClientUseConsumable( CEconItemView *pItem, vgui::Panel *pParent ) const;
#endif // CLIENT_DLL
};

#endif // ECONITEMTOOLS_H
