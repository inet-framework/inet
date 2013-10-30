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

#include "OSPFRouting.h"

#include "MessageHandler.h"
#include "OSPFConfigReader.h"
#include "IPv4RoutingTableAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "IPSocket.h"


Define_Module(OSPFRouting);


OSPFRouting::OSPFRouting()
{
    isUp = false;
    ospfRouter = NULL;
}

OSPFRouting::~OSPFRouting()
{
    delete ospfRouter;
}

void OSPFRouting::initialize(int stage)
{
    InetSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_OSPF);

        isUp = isNodeUp();
        if (isUp)
            createOspfRouter();
    }
}

void OSPFRouting::createOspfRouter()
{
    IIPv4RoutingTable *rt = IPv4RoutingTableAccess().get();
    ospfRouter = new OSPF::Router(rt->getRouterId(), this);

    // read the OSPF AS configuration
    cXMLElement *ospfConfig = par("ospfConfig").xmlValue();
    OSPFConfigReader configReader(this);
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
    EV << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool OSPFRouting::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter == NULL);
            isUp = true;
            createOspfRouter();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_ROUTING_PROTOCOLS) {
            ASSERT(ospfRouter);
            isUp = false;
            delete ospfRouter;
            ospfRouter = NULL;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            ASSERT(ospfRouter);
            isUp = false;
            delete ospfRouter;
            ospfRouter = NULL;
        }
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }
    return true;
}

bool OSPFRouting::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void OSPFRouting::insertExternalRoute(int ifIndex, const OSPF::IPv4AddressRange &netAddr)
{
    simulation.setContext(this);
    OSPFASExternalLSAContents newExternalContents;
    newExternalContents.setRouteCost(OSPF_BGP_DEFAULT_COST);
    newExternalContents.setExternalRouteTag(OSPF_EXTERNAL_ROUTES_LEARNED_BY_BGP);
    const IPv4Address netmask = netAddr.mask;
    newExternalContents.setNetworkMask(netmask);
    ospfRouter->updateExternalRoute(netAddr.address, newExternalContents, ifIndex);
}

bool OSPFRouting::checkExternalRoute(const IPv4Address &route)
{
    for (unsigned long i=1; i < ospfRouter->getASExternalLSACount(); i++)
    {
        OSPF::ASExternalLSA* externalLSA = ospfRouter->getASExternalLSA(i);
        IPv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) //FIXME was this meant???
            return true;
    }
    return false;
}

