//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/bgpv4/Bgp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/routing/bgpv4/BgpConfigReader.h"
#include "inet/routing/bgpv4/BgpSession.h"

namespace inet {
namespace bgp {

Define_Module(Bgp);

Bgp::Bgp()
{
}

Bgp::~Bgp()
{
    cancelAndDelete(startupTimer);
    delete bgpRouter;
}

void Bgp::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        startupTimer = new cMessage("BGP-startup");
    }
}

void Bgp::finish()
{
    if (!bgpRouter) {
        EV_ERROR << "Protocol is turned off. \n";
        return;
    }

    bgpRouter->recordStatistics();
}

void Bgp::handleMessageWhenUp(cMessage *msg)
{
    if (msg == startupTimer)
        createBgpRouter();
    else {
        if (!bgpRouter) {
            if (msg->isSelfMessage())
                throw cRuntimeError("Model error: self msg '%s' received before BGP startup", msg->getName());
            EV_WARN << "BGP has not started yet, dropping '" << msg->getName() << "' message\n";
            delete msg;
        }
        else if (msg->isSelfMessage()) // BGP level
            handleTimer(msg);
        else if (!strcmp(msg->getArrivalGate()->getName(), "socketIn")) // TCP level
            bgpRouter->processMessageFromTCP(msg);
        else
            delete msg;
    }
}

void Bgp::handleStartOperation(LifecycleOperation *operation)
{
    startBgp();
}

void Bgp::handleStopOperation(LifecycleOperation *operation)
{
    stopBgp(false);
}

void Bgp::handleCrashOperation(LifecycleOperation *operation)
{
    stopBgp(true);
}

void Bgp::startBgp()
{
    ASSERT(bgpRouter == nullptr);
    simtime_t startupTime = par("startupTime");
    if (startupTime == 0)
        createBgpRouter();
    else
        scheduleAfter(startupTime, startupTimer);
}

void Bgp::stopBgp(bool abort)
{
    cancelEvent(startupTimer);
    removeBgpRoutes();
    if (bgpRouter)
        bgpRouter->closeSessions(abort);
    delete bgpRouter;
    bgpRouter = nullptr;
}

void Bgp::removeBgpRoutes()
{
    for (int i = rt->getNumRoutes() - 1; i >= 0; i--) {
        Ipv4Route *route = rt->getRoute(i);
        if (route->getSourceType() == IRoute::BGP) {
            EV_INFO << "Removing BGP route " << route->str() << endl;
            rt->deleteRoute(route);
        }
    }
}

void Bgp::createBgpRouter()
{
    ASSERT(bgpRouter == nullptr);
    bgpRouter = new BgpRouter(this, ift, rt);

    // read BGP configuration
    cXMLElement *bgpConfig = par("bgpConfig");
    BgpConfigReader configReader(this, ift);
    configReader.loadConfigFromXML(bgpConfig, bgpRouter);

    bgpRouter->printSessionSummary();
    bgpRouter->addWatches();
}

void Bgp::handleTimer(cMessage *timer)
{
    BgpSession *pSession = (BgpSession *)timer->getContextPointer();
    if (pSession) {
        switch (timer->getKind()) {
            case START_EVENT_KIND:
                EV_INFO << "Processing Start Event" << std::endl;
                pSession->getFSM()->ManualStart();
                break;

            case CONNECT_RETRY_KIND:
                EV_INFO << "Expiring Connect Retry Timer" << std::endl;
                pSession->getFSM()->ConnectRetryTimer_Expires();
                break;

            case HOLD_TIME_KIND:
                EV_INFO << "Expiring Hold Timer" << std::endl;
                pSession->getFSM()->HoldTimer_Expires();
                break;

            case KEEP_ALIVE_KIND:
                EV_INFO << "Expiring Keep Alive timer" << std::endl;
                pSession->getFSM()->KeepaliveTimer_Expires();
                break;

            default:
                throw cRuntimeError("Invalid timer kind %d", timer->getKind());
        }
    }
}

} // namespace bgp
} // namespace inet
