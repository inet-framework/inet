//
// Copyright (C) 2006 Andras Babos and Andras Varga
// Copyright (C) 2012 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <string>
#include <map>
#include <stdlib.h>
#include <memory.h>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/routing/ospfv2/Ospf.h"

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/OspfConfigReader.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace ospf {

Define_Module(Ospf);

Ospf::Ospf()
{
}

Ospf::~Ospf()
{
    delete ospfRouter;
}

void Ospf::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        isUp = isNodeUp();
        if (isUp)
            createOspfRouter();
        registerService(Protocol::ospf, nullptr, gate("ipIn"));
        registerProtocol(Protocol::ospf, gate("ipOut"), nullptr);
    }
}

void Ospf::createOspfRouter()
{
    ospfRouter = new Router(rt->getRouterId(), this, ift, rt);

    // read the OSPF AS configuration
    cXMLElement *ospfConfig = par("ospfConfig");
    OspfConfigReader configReader(this, ift);
    if (!configReader.loadConfigFromXML(ospfConfig, ospfRouter))
        throw cRuntimeError("Error reading AS configuration from %s", ospfConfig->getSourceLocation());

    ospfRouter->addWatches();
}

void Ospf::handleMessage(cMessage *msg)
{
    if (!isUp)
        handleMessageWhenDown(msg);
    else
        ospfRouter->getMessageHandler()->messageReceived(msg);
}

void Ospf::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
    EV_ERROR << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool Ospf::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter == nullptr);
            isUp = true;
            createOspfRouter();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter);
            isUp = false;
            delete ospfRouter;
            ospfRouter = nullptr;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH) {
            ASSERT(ospfRouter);
            isUp = false;
            delete ospfRouter;
            ospfRouter = nullptr;
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }
    return true;
}

bool Ospf::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void Ospf::insertExternalRoute(int ifIndex, const Ipv4AddressRange& netAddr)
{
    Enter_Method_Silent();
    OspfAsExternalLsaContents newExternalContents;
    newExternalContents.setRouteCost(OSPF_BGP_DEFAULT_COST);
    newExternalContents.setExternalRouteTag(OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP);
    const Ipv4Address netmask = netAddr.mask;
    newExternalContents.setNetworkMask(netmask);
    ospfRouter->updateExternalRoute(netAddr.address, newExternalContents, ifIndex);
}

bool Ospf::checkExternalRoute(const Ipv4Address& route)
{
    Enter_Method_Silent();
    for (unsigned long i = 1; i < ospfRouter->getASExternalLSACount(); i++) {
        AsExternalLsa *externalLSA = ospfRouter->getASExternalLSA(i);
        Ipv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) //FIXME was this meant???
            return true;
    }
    return false;
}

} // namespace ospf

} // namespace inet

