//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/bgpv4/Bgp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
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
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);

        startupTimer = new cMessage("BGP-startup");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) { // interfaces and static routes are already initialized
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        isUp = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
        if (isUp) {
            simtime_t startupTime = par("startupTime");
            if (startupTime == 0)
                createBgpRouter();
            else
                scheduleAfter(startupTime, startupTimer);
        }
    }
}

void Bgp::finish()
{
    if (!isUp) {
        EV_ERROR << "Protocol is turned off. \n";
        return;
    }

    bgpRouter->recordStatistics();
}

void Bgp::handleMessage(cMessage *msg)
{
    if (!isUp) {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
        EV_ERROR << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
        delete msg;
        return;
    }

    if (msg == startupTimer)
        createBgpRouter();
    else if (msg->isSelfMessage()) // BGP level
        handleTimer(msg);
    else if (!strcmp(msg->getArrivalGate()->getName(), "socketIn")) // TCP level
        bgpRouter->processMessageFromTCP(msg);
    else
        delete msg;
}

void Bgp::createBgpRouter()
{
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

