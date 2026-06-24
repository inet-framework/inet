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
    cancelAndDelete(shutdownTimer);
    delete bgpRouter;
}

void Bgp::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        const char *addressFamily = par("addressFamily");
        if (!strcmp(addressFamily, "ipv4"))
            networkProtocol = &Protocol::ipv4;
        else if (!strcmp(addressFamily, "ipv6"))
            networkProtocol = &Protocol::ipv6;
        else
            throw cRuntimeError("Bgp: invalid addressFamily '%s' (must be 'ipv4' or 'ipv6')", addressFamily);

        startupTimer = new cMessage("BGP-startup");
        shutdownTimer = new cMessage("BGP-shutdown");
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
    else if (msg == shutdownTimer) {
        // Graceful stop: TCP has had shutdownTime to flush the session teardown; now destroy
        // the BGP state and let the lifecycle stop operation finish.
        ASSERT(operationalState == State::STOPPING_OPERATION);
        delete bgpRouter;
        bgpRouter = nullptr;
        finishActiveOperation();
    }
    else if (operationalState == State::STOPPING_OPERATION) {
        // Graceful shutdown window: sessions are already closing and TCP finishes the teardown
        // on its own. Ignore further BGP timers / TCP indications (no reconnect, no re-close);
        // the BGP state is destroyed when shutdownTimer fires. Self-messages are owned by
        // bgpRouter and cancelAndDelete'd then, so only delete foreign (TCP) messages here.
        if (!msg->isSelfMessage())
            delete msg;
    }
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
            bgpRouter->processMessageFromTcp(msg);
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
    // Graceful shutdown: close the sessions (TCP FIN) and give TCP shutdownTime to flush the
    // teardown before the BGP state is destroyed (in the shutdownTimer handler). The stop
    // operation finishes asynchronously, mirroring Rip::handleStopOperation.
    cancelEvent(startupTimer);
    removeBgpRoutes();
    if (bgpRouter) {
        bgpRouter->closeSessions(false);
        scheduleAfter(par("shutdownTime"), shutdownTimer);
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
    // else: BGP not started yet, nothing to flush; the operation finishes synchronously.
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
    cancelEvent(shutdownTimer);
    removeBgpRoutes();
    if (bgpRouter)
        bgpRouter->closeSessions(abort);
    delete bgpRouter;
    bgpRouter = nullptr;
}

void Bgp::removeBgpRoutes()
{
    for (int i = rt->getNumRoutes() - 1; i >= 0; i--) {
        IRoute *route = rt->getRoute(i);
        if (route->getSourceType() == IRoute::BGP) {
            EV_INFO << "Removing BGP route to " << route->getDestinationAsGeneric()
                    << "/" << route->getPrefixLength() << endl;
            rt->deleteRoute(route);
        }
    }
}

void Bgp::createBgpRouter()
{
    ASSERT(bgpRouter == nullptr);
    bgpRouter = new BgpRouter(this, ift, rt, networkProtocol);

    // read BGP configuration
    cXMLElement *bgpConfig = par("bgpConfig");
    BgpConfigReader configReader(this, ift);
    configReader.loadConfigFromXml(bgpConfig, bgpRouter);

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
                pSession->getFsm()->ManualStart();
                break;

            case CONNECT_RETRY_KIND:
                EV_INFO << "Expiring Connect Retry Timer" << std::endl;
                pSession->getFsm()->ConnectRetryTimer_Expires();
                break;

            case HOLD_TIME_KIND:
                EV_INFO << "Expiring Hold Timer" << std::endl;
                pSession->getFsm()->HoldTimer_Expires();
                break;

            case KEEP_ALIVE_KIND:
                EV_INFO << "Expiring Keep Alive timer" << std::endl;
                pSession->getFsm()->KeepaliveTimer_Expires();
                break;

            default:
                throw cRuntimeError("Invalid timer kind %d", timer->getKind());
        }
    }
}

} // namespace bgp
} // namespace inet
