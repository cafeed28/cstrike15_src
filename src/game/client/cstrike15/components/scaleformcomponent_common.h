#pragma once

#include "uicomponents/uicomponent_common.h"
#include "keyvalues.h"
#include "xbox/xboxstubs.h"

UI_COMPONENT_DECLARE_EVENT0(Blog, HideBlog); // TODO
UI_COMPONENT_DECLARE_EVENT0(Blog, ShowBlog); // TODO
UI_COMPONENT_DECLARE_EVENT0(Device, Reset); // TODO
UI_COMPONENT_DECLARE_EVENT0(Device, Tick); // TODO
UI_COMPONENT_DECLARE_EVENT0(EmbeddedStream, VideoPlaying); // TODO
UI_COMPONENT_DECLARE_EVENT0(EmbeddedStream, VolumeChanged); // TODO
UI_COMPONENT_DECLARE_EVENT1(FriendsList, NameChanged, "xuid", XUID);
UI_COMPONENT_DECLARE_EVENT0(FriendsList, ProfileUpdated); // TODO
UI_COMPONENT_DECLARE_EVENT0(FriendsList, RebuildFriendsList); // TODO
UI_COMPONENT_DECLARE_EVENT0(GC, Hello);
UI_COMPONENT_DECLARE_EVENT0(GlobalGame, StateChange); // TODO
UI_COMPONENT_DECLARE_EVENT0(ImageCache, PurgePassEnd); // TODO
UI_COMPONENT_DECLARE_EVENT0(ImageCache, PurgePassStart); // TODO
UI_COMPONENT_DECLARE_EVENT0(Inventory, CrateOpened); // TODO
UI_COMPONENT_DECLARE_EVENT0(Inventory, ModelPanelReady); // TODO
UI_COMPONENT_DECLARE_EVENT0(Inventory, OnHasNewItems); // TODO
UI_COMPONENT_DECLARE_EVENT0(Inventory, WeaponPreviewRequest); // TODO
UI_COMPONENT_DECLARE_EVENT0(Leaderboards, StateChange); // TODO
UI_COMPONENT_DECLARE_EVENT0(MatchInfo, StateChange); // TODO
UI_COMPONENT_DECLARE_EVENT0(MatchList, FantasyUploaded); // TODO
UI_COMPONENT_DECLARE_EVENT0(MatchList, PredictionUploaded); // TODO
UI_COMPONENT_DECLARE_EVENT0(MatchList, StateChange); // TODO
UI_COMPONENT_DECLARE_EVENT0(MyPersona, AccountPrivacySettingsChanged);
UI_COMPONENT_DECLARE_EVENT2(MyPersona, ClientAuthKeyCode, "eventid", int, "code", const char*);
UI_COMPONENT_DECLARE_EVENT0(MyPersona, ElevatedStateUpdate);
UI_COMPONENT_DECLARE_EVENT0(MyPersona, InventoryUpdated);
UI_COMPONENT_DECLARE_EVENT2(MyPersona, InventoryUpdatedImmediate, "type", int, "econitemptr", void*);
UI_COMPONENT_DECLARE_EVENT0(MyPersona, MedalsChanged);
UI_COMPONENT_DECLARE_EVENT0(MyPersona, NameChanged);
UI_COMPONENT_DECLARE_EVENT0(News, BlogUpdate); // TODO
UI_COMPONENT_DECLARE_EVENT0(Overwatch, CaseUpdated); // TODO
UI_COMPONENT_DECLARE_EVENT0(PartyBrowser, InviteConsumed); // TODO
UI_COMPONENT_DECLARE_EVENT0(PartyBrowser, InviteReceived); // TODO
UI_COMPONENT_DECLARE_EVENT0(PartyBrowser, Refresh); // TODO
UI_COMPONENT_DECLARE_EVENT0(PartyList, MatchmakingSessionUpdate); // TODO
UI_COMPONENT_DECLARE_EVENT0(PartyList, RebuildPartyList); // TODO
UI_COMPONENT_DECLARE_EVENT0(Store, PriceSheetChanged); // TODO
UI_COMPONENT_DECLARE_EVENT0(Store, PurchaseCompleted); // TODO
UI_COMPONENT_DECLARE_EVENT0(Store, UpdateStoreTiles); // TODO
UI_COMPONENT_DECLARE_EVENT0(Streams, MyTwitchTvStateUpdate); // TODO
UI_COMPONENT_DECLARE_EVENT0(Streams, PreviewImageUpdate); // TODO
UI_COMPONENT_DECLARE_EVENT0(Streams, Update); // TODO
UI_COMPONENT_DECLARE_EVENT0(Teammates, Refresh); // TODO
UI_COMPONENT_DECLARE_EVENT0(TournamentMatch, DraftUpdate); // TODO
UI_COMPONENT_DECLARE_EVENT0(UGC, DownloadingStateUpdate); // TODO