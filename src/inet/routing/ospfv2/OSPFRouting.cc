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

#include "inet/routing/ospfv2/OSPFRouting.h"

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/OSPFConfigReader.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace ospf {

Define_Module(OSPFRouting);

OSPFRouting::OSPFRouting()
{
}

OSPFRouting::~OSPFRouting()
{
    delete ospfRouter;
}

void OSPFRouting::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_OSPF);

        isUp = isNodeUp();
        if (isUp)
            createOspfRouter();
    }
}

void OSPFRouting::createOspfRouter()
{
    ospfRouter = new Router(rt->getRouterId(), this, ift, rt);

    // read the OSPF AS configuration
    cXMLElement *ospfConfig = par("ospfConfig").xmlValue();
    OSPFConfigReader configReader(this, ift);
    if (!configReader.loadConfigFromXML(ospfConfig, ospfRouter))
        throw cRuntimeError("Error reading AS configuration from %s", ospfConfig->getSourceLocation());

    ospfRouter->addWatches();
}

void OSPFRouting::handleMessage(cMessage *msg)
{
    if (!isUp)
        handleMessageWhenDown(msg);
    else
        ospfRouter->getMessageHandler()->messageReceived(msg);
}

void OSPFRouting::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
    EV_ERROR << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool OSPFRouting::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter == nullptr);
            isUp = true;
            createOspfRouter();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter);
            isUp = false;
            delete ospfRouter;
            ospfRouter = nullptr;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
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

bool OSPFRouting::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void OSPFRouting::insertExternalRoute(int ifIndex, const IPv4AddressRange& netAddr)
{
    Enter_Method_Silent();
    OSPFASExternalLSAContents newExternalContents;
    newExternalContents.setRouteCost(OSPF_BGP_DEFAULT_COST);
    newExternalContents.setExternalRouteTag(OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP);
    const IPv4Address netmask = netAddr.mask;
    newExternalContents.setNetworkMask(netmask);
    ospfRouter->updateExternalRoute(netAddr.address, newExternalContents, ifIndex);
}

bool OSPFRouting::checkExternalRoute(const IPv4Address& route)
{
    Enter_Method_Silent();
    for (unsigned long i = 1; i < ospfRouter->getASExternalLSACount(); i++) {
        ASExternalLSA *externalLSA = ospfRouter->getASExternalLSA(i);
        IPv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) //FIXME was this meant???
            return true;
    }
    return false;
}

} // namespace ospf

} // namespace inet

