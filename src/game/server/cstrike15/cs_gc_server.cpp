#include "cbase.h"
#include "cs_gc_server.h"
#include "base_gcmessages.pb.h"
#include "steam/steam_api.h"
#include "steam/isteamappdisableupdate.h"
#include "filesystem.h"
#include "gameinterface.h"

#include "matchmaking/imatchframework.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CServerGameDLL g_ServerGameDLL;

using namespace GCSDK;

static CCSGCServerSystem s_CSGCServerSystem;
CCSGCServerSystem* CSGCServerSystem() { return &s_CSGCServerSystem; }

CCSGCServerSystem::CCSGCServerSystem()
{
    SetGCClientSystem(this);
}

CCSGCServerSystem::~CCSGCServerSystem()
{
    SetGCClientSystem(NULL);
}

bool CCSGCServerSystem::Init()
{
    // The retail build does nothing here.

    // Register the CSGO-specific messages with the gcsdk so it can log the names.
    GCSDK::MsgRegistrationFromEnumDescriptor(ECsgoGCMsg_descriptor(), GCSDK::MT_GC);
    return true;
}

void CCSGCServerSystem::PostInit()
{
    BaseClass::PostInit();
}

ISteamAppDisableUpdate* g_pSteamAppDisableUpdate = NULL;

void CCSGCServerSystem::PreInitGC()
{
}

void CCSGCServerSystem::PostInitGC()
{
    /* Do nothing */
}

void CCSGCServerSystem::LevelInitPreEntity()
{
    BaseClass::LevelInitPreEntity();
}

void CCSGCServerSystem::LevelShutdownPostEntity()
{
    BaseClass::LevelShutdownPostEntity();
}

void CCSGCServerSystem::Shutdown()
{
    BaseClass::Shutdown();
}

void CCSGCServerSystem::PreClientUpdate()
{
    BaseClass::Shutdown();
    CRTime::UpdateRealTime();
}

class CGCClientJobServerWelcome : public GCSDK::CGCClientJob
{
public:
    CGCClientJobServerWelcome(GCSDK::CGCClient* pGCServer) : GCSDK::CGCClientJob(pGCServer) {}

    virtual bool BYieldingRunJobFromMsg(IMsgNetPacket* pNetPacket)
    {
        GCSDK::CProtoBufMsg<CMsgClientWelcome> msgClientWelcome(pNetPacket);

        int g_gcServerVersion = msgClientWelcome.Body().version();
        int nVersion = engine->GetServerVersion();
        if (g_gcServerVersion && nVersion && nVersion != g_gcServerVersion)
        {
            Msg("Version out of date (GC wants %d, we are %d)!\n", g_gcServerVersion, engine->GetServerVersion());
        }
        else
        {
            //if (CSGCServerSystem().m_nUnk)
                //CSGCServerSystem().m_nUnk = 0;
            ++CSGCServerSystem()->m_nSessionInstanceIndex;

            CSGCServerSystem()->ProcessWelcomeMessage(msgClientWelcome.Body());

            Msg("GC Connection established for server version %d, instance idx %d\n", engine->GetServerVersion(), CSGCServerSystem()->m_nSessionInstanceIndex);

            CMsgCStrike15Welcome msgCSWelcome;
            if (msgCSWelcome.ParseFromString(msgClientWelcome.Body().game_data()))
            {
                if (msgCSWelcome.has_gscookieid() && msgCSWelcome.gscookieid())
                {
                    static uint64 g_GsCookie = msgCSWelcome.gscookieid();

                    char szReservationPayload[64] = { 0 };
                    V_sprintf_safe(szReservationPayload, "R%p", &g_GsCookie);

                    engine->ReserveServerForQueuedGame(szReservationPayload);
                }

                //GCCStrikeWelcomeMessageReceived(msgCSWelcome);
            }

            g_ServerGameDLL.UpdateGCInformation();
        }
        return true;
    }
};
GC_REG_JOB(GCSDK::CGCClient, CGCClientJobServerWelcome, "CGCClientJobServerWelcome", k_EMsgGCServerWelcome, k_EServerTypeGCClient);