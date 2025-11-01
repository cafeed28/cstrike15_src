#include "cbase.h"
#include "cs_gc_client.h"
#include "base_gcmessages.pb.h"
#include "steam/steam_api.h"
#include "steam/isteamappdisableupdate.h"
#include "filesystem.h"

#include "matchmaking/imatchframework.h"
#include "components/scaleformcomponent_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char* g_szConfirmedReservationFilename = "qmmconnect.dt";

void ConfirmedReservationData_t::SetPacketData(CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve& msg) const
{
    msg.set_reservationid(m_nReservationId);
    msg.set_server_address(ns_address_render(m_serverAddress).String());

    if (m_nServerId.IsValid())
    {
        msg.set_serverid(m_nServerId.ConvertToUint64());
    }

    if (m_directUdp.IsValid())
    {
        msg.set_direct_udp_ip(m_directUdp.GetIPHostByteOrder());
        msg.set_direct_udp_port(m_directUdp.GetPort());
    }

    if (m_map.empty())
    {
        msg.set_map(m_map);
    }
}

void ConfirmedReservationData_t::SetReservationData(const CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve& msg)
{
    m_nReservationId = msg.reservationid();
    m_serverAddress.SetFromString(msg.server_address().c_str());
    m_nServerId = msg.serverid();
    m_directUdp.SetIP(msg.direct_udp_port());
    m_directUdp.SetPort(msg.direct_udp_port());
    m_map = msg.map();
}

void ConfirmedReservationData_t::Clear() const
{
    if (g_pFullFileSystem->FileExists(g_szConfirmedReservationFilename))
    {
        g_pFullFileSystem->RemoveFile(g_szConfirmedReservationFilename);
    }
}

void ConfirmedReservationData_t::Write() const
{
    FileHandle_t file = g_pFullFileSystem->Open(g_szConfirmedReservationFilename, "wb");
    if (file)
    {
        CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve msg;
        SetPacketData(msg);
        std::string strMsg = msg.SerializeAsString();
        g_pFullFileSystem->Write(strMsg.c_str(), strMsg.size(), file);
        g_pFullFileSystem->Close(file);
    }
}

bool ConfirmedReservationData_t::Load()
{
    CUtlBuffer buf;
    if (g_pFullFileSystem->ReadFile(g_szConfirmedReservationFilename, NULL, buf))
    {
        CMsgGCCStrike15_v2_MatchmakingGC2ClientReserve v4;
        if (v4.ParseFromArray(buf.PeekGet(), buf.TellPut() - buf.TellGet()))
        {
            SetReservationData(v4);
            return true;
        }
    }

    return false;
}

using namespace GCSDK;

static CCSGCClientSystem s_CSGCClientSystem;
CCSGCClientSystem * CSGCClientSystem() { return &s_CSGCClientSystem; }

CCSGCClientSystem::CCSGCClientSystem()
{
    SetGCClientSystem(this);
}

CCSGCClientSystem::~CCSGCClientSystem()
{
    SetGCClientSystem(NULL);
}

bool CCSGCClientSystem::Init()
{
    // The retail build does nothing here.

    // Register the CSGO-specific messages with the gcsdk so it can log the names.
    GCSDK::MsgRegistrationFromEnumDescriptor(ECsgoGCMsg_descriptor(), GCSDK::MT_GC);
    return true;
}

void CCSGCClientSystem::PostInit()
{
    BaseClass::PostInit();
}

ISteamAppDisableUpdate* g_pSteamAppDisableUpdate = NULL;

void CCSGCClientSystem::PreInitGC()
{
    if (steamapicontext)
    {
        if (steamapicontext->SteamClient())
        {
            HSteamUser hSteamUser = SteamAPI_GetHSteamUser();
            HSteamUser hSteamPipe = SteamAPI_GetHSteamPipe();
            g_pSteamAppDisableUpdate = (ISteamAppDisableUpdate*)steamapicontext->SteamClient()->GetISteamGenericInterface(hSteamUser, hSteamPipe, STEAMAPPDISABLEUPDATE_INTERFACE_VERSION);
        }
    }

    UpdateSteamAppDisableUpdate();
}

void CCSGCClientSystem::PostInitGC()
{
    /* Do nothing */
}

void CCSGCClientSystem::LevelInitPreEntity()
{
    BaseClass::LevelInitPreEntity();
}

void CCSGCClientSystem::LevelShutdownPostEntity()
{
    BaseClass::LevelShutdownPostEntity();
}

void CCSGCClientSystem::Shutdown()
{
    BaseClass::Shutdown();
}

void CCSGCClientSystem::Update(float frametime)
{
    BaseClass::Update(frametime);
}

void CCSGCClientSystem::GetTimePlayedConsecutively() const {}

void CCSGCClientSystem::UpdateSteamAppDisableUpdate() {}

bool CCSGCClientSystem::OnReceivedWelcomeMessage(CMsgClientWelcome& msgClientWelcome, CMsgCStrike15Welcome& msgCSWelcome)
{
    m_nTimePlayedConsecutively = msgCSWelcome.timeplayedconsecutively();
    m_nLastTimePlayed = msgCSWelcome.last_time_played();
    
    m_ip.SetIP(msgCSWelcome.last_ip_address());
    m_ip.SetType(NA_IP);

    return true;
}

CMsgGCCStrike15_v2_MatchmakingGC2ClientHello g_GC2ClientHello; // RE TODO: move to more appropriate

void ReceiveMatchmakingHelloMessage(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msg)
{
    int vac_banned = 0;
    int rank_id = 0;
    int wins = 0;

    if (msg.has_ranking())
    {
        rank_id = msg.ranking().rank_id();
        wins = msg.ranking().wins();
    }
    if (msg.has_vac_banned())
    {
        vac_banned = msg.vac_banned();
    }

    int cmd_friendly = 0;
    int cmd_teaching = 0;
    int cmd_leader = 0;
    if (msg.has_commendation())
    {
        cmd_friendly = msg.commendation().cmd_friendly();
        cmd_teaching = msg.commendation().cmd_teaching();
        cmd_leader = msg.commendation().cmd_leader();
    }

    DevMsg("Client hello received: %x/%s (%u/%u sec) R%u W%u V%u C[%u-%u-%u]\n",
        msg.account_id(),
        msg.has_ongoingmatch() ? "ongoingmatch" : "ok",
        msg.penalty_reason(),
        msg.penalty_seconds(),
        rank_id,
        wins,
        vac_banned,
        cmd_friendly,
        cmd_teaching,
        cmd_leader
    );

    if (msg.has_my_current_team())
    {
        const char* team_name = msg.my_current_team().has_team_name() ? msg.my_current_team().team_name().c_str() : "";
        const char* team_tag = msg.my_current_team().has_team_tag() ? msg.my_current_team().team_tag().c_str() : "";
        const char* team_flag = msg.my_current_team().has_team_flag() ? msg.my_current_team().team_flag().c_str() : "";
        DevMsg("Client registered for tournament team '%s' (%s-%s-%u)\n", team_name, team_tag, team_flag, msg.my_current_team().team_id());
        if (msg.has_my_current_event())
        {
            const char* event_name = msg.my_current_event().has_event_name() ? msg.my_current_event().event_name().c_str() : "";
            const char* event_tag = msg.my_current_event().has_event_tag() ? msg.my_current_event().event_tag().c_str() : "";
            DevMsg("Tournament event is active '%s' [%s] %u\n", event_name, event_tag, msg.my_current_event().event_id());
        }
    }

    if (msg.my_current_event_teams_size())
    {
        //V_qsort_s(msg.mutable_my_current_event_teams(), msg.my_current_event_teams_size(), sizeof(void*), CompareTeamsByName);
    }

    if (msg.has_penalty_seconds())
    {
        struct tm t;
        Plat_GetLocalTime(&t);
        msg.set_penalty_seconds(msg.penalty_seconds() + Plat_timegm(&t) + 1);
    }

    if (msg.has_ongoingmatch())
    {
        ConfirmedReservationData_t data;
        data.SetReservationData(msg.ongoingmatch());
        data.Write();
    }
    else
    {
        ConfirmedReservationData_t data;
        data.Clear();
    }

    int match_id = 0;
    if (g_GC2ClientHello.has_ongoingmatch())
    {
        if (g_GC2ClientHello.ongoingmatch().has_reservation())
        {
            if (g_GC2ClientHello.ongoingmatch().reservation().match_id())
            {
                match_id = g_GC2ClientHello.ongoingmatch().reservation().match_id();
            }
        }
    }

    g_GC2ClientHello.CopyFrom(msg);

    //ShowGlobalServerStats();
    //CStorePriceSheetHandler::GetStorePriceSheetHandler();
    UI_COMPONENT_BROADCAST_EVENT(GC, Hello);
    
    if (msg.has_ongoingmatch())
    {
        if (msg.ongoingmatch().has_reservation())
        {
            if (msg.ongoingmatch().reservation().match_id())
            {
                match_id = msg.ongoingmatch().reservation().match_id();
            }
        }
    }

    if (match_id)
    {
        //g_uiLastCompetitiveMatchId = match_id;
        //g_timeLastCompetitiveMatchIdUnregistered = Plat_FloatTime();
    }
}

bool CCSGCClientSystem::OnReceivedMatchmakingWelcomeMessage(CMsgGCCStrike15_v2_MatchmakingGC2ClientHello& msg)
{
    ReceiveMatchmakingHelloMessage(msg);
    return true;
}

class CGCClientWelcomeJob : public GCSDK::CGCClientJob
{
public:
    CGCClientWelcomeJob(GCSDK::CGCClient* pGCClient) : GCSDK::CGCClientJob(pGCClient) {}

    virtual bool BYieldingRunGCJob(IMsgNetPacket* pNetPacket)
    {
        GCSDK::CProtoBufMsg<CMsgClientWelcome> msgClientWelcome(pNetPacket);

        CMsgCStrike15Welcome msgCSWelcome;
        msgCSWelcome.ParseFromString(msgClientWelcome.Body().game_data());

        if (CSGCClientSystem()->OnReceivedWelcomeMessage(msgClientWelcome.Body(), msgCSWelcome))
        {
            CSGCClientSystem()->ProcessWelcomeMessage(msgClientWelcome.Body());

            CMsgGCCStrike15_v2_MatchmakingGC2ClientHello msgGC2ClientHello;
            msgGC2ClientHello.ParseFromString(msgClientWelcome.Body().game_data2());
            
            CSGCClientSystem()->OnReceivedMatchmakingWelcomeMessage(msgGC2ClientHello);
        }
        return true;
    }
};

class ClientJob_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello : public GCSDK::CGCClientJob
{
public:
    ClientJob_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello(GCSDK::CGCClient* pGCClient) : GCSDK::CGCClientJob(pGCClient) {}

    virtual bool BYieldingRunGCJob(IMsgNetPacket* pNetPacket)
    {
        GCSDK::CProtoBufMsg<CMsgGCCStrike15_v2_MatchmakingGC2ClientHello> msg(pNetPacket);
        ReceiveMatchmakingHelloMessage(msg.Body());
        return true;
    }
};

GC_REG_JOB(GCSDK::CGCClient, CGCClientWelcomeJob, "CGCClientWelcomeJob", k_EMsgGCClientWelcome, GCSDK::k_EServerTypeGCClient);
GC_REG_JOB(GCSDK::CGCClient, ClientJob_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello, "ClientJob_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello", k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello, GCSDK::k_EServerTypeGCClient);
//GC_REG_JOB(GCSDK::CGCClient, CGCClientJobSteamDatagramTicket, "CGCClientJobSteamDatagramTicket", k_EMsgGCClientWelcome, GCSDK::k_EServerTypeGCClient);