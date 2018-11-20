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
    cancelAndDelete(startupTimer);
    delete ospfRouter;
}

void Ospf::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        startupTimer = new cMessage("OSPF-startup");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {  // interfaces and static routes are already initialized
        registerService(Protocol::ospf, nullptr, gate("ipIn"));
        registerProtocol(Protocol::ospf, gate("ipOut"), nullptr);
    }
}

void Ospf::handleMessageWhenUp(cMessage *msg)
{
    if (msg == startupTimer) {
        createOspfRouter();
        subscribe();
    }
    else
        ospfRouter->getMessageHandler()->messageReceived(msg);

}

void Ospf::createOspfRouter()
{
    ospfRouter = new Router(this, ift, rt);

    // read the OSPF AS configuration
    cXMLElement *ospfConfig = par("ospfConfig");
    OspfConfigReader configReader(this, ift);
    if (!configReader.loadConfigFromXML(ospfConfig, ospfRouter))
        throw cRuntimeError("Error reading AS configuration from %s", ospfConfig->getSourceLocation());

    ospfRouter->addWatches();
}

void Ospf::subscribe()
{
    host->subscribe(interfaceCreatedSignal, this);
    host->subscribe(interfaceDeletedSignal, this);
    host->subscribe(interfaceStateChangedSignal, this);
}

void Ospf::unsubscribe()
{
    host->unsubscribe(interfaceCreatedSignal, this);
    host->unsubscribe(interfaceDeletedSignal, this);
    host->unsubscribe(interfaceStateChangedSignal, this);
}

/**
 * Listen on interface changes and update private data structures.
 */
void Ospf::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("Ospf::receiveSignal");

    const InterfaceEntry *ie;
    const InterfaceEntryChangeDetails *change;

    if (signalID == interfaceCreatedSignal) {
        // configure interface for RIP
        ie = check_and_cast<const InterfaceEntry *>(obj);
        if (ie->isMulticast() && !ie->isLoopback()) {
            // TODO
        }
    }
    else if (signalID == interfaceDeletedSignal) {
        ie = check_and_cast<const InterfaceEntry *>(obj);
        // TODO
    }
    else if (signalID == interfaceStateChangedSignal) {
        change = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        if (change->getFieldId() == InterfaceEntry::F_CARRIER || change->getFieldId() == InterfaceEntry::F_STATE) {
            ie = change->getInterfaceEntry();
            if (!ie->isUp())
                handleInterfaceDown(ie);
            else {
                // interface went back online. Do nothing!
                // Wait for Hello messages to establish adjacency.
            }
        }
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

bool Ospf::handleNodeStart(IDoneCallback *)
{
    ASSERT(ospfRouter == nullptr);
    simtime_t startupTime = par("startupTime");
    if (startupTime <= simTime()) {
        createOspfRouter();
        subscribe();
    }
    else
        scheduleAt(simTime() + startupTime, startupTimer);
    return true;
}

bool Ospf::handleNodeShutdown(IDoneCallback *)
{
    ASSERT(ospfRouter);
    delete ospfRouter;
    cancelEvent(startupTimer);
    ospfRouter = nullptr;
    unsubscribe();
    return true;
}

void Ospf::handleNodeCrash()
{
    ASSERT(ospfRouter);
    delete ospfRouter;
    cancelEvent(startupTimer);
    ospfRouter = nullptr;
    unsubscribe();
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
    for (uint32_t i = 0; i < ospfRouter->getASExternalLSACount(); i++) {
        AsExternalLsa *externalLSA = ospfRouter->getASExternalLSA(i);
        Ipv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) //FIXME was this meant???
            return true;
    }
    return false;
}

void Ospf::handleInterfaceDown(const InterfaceEntry *ie)
{
    EV_DEBUG << "interface " << ie->getInterfaceId() << " went down. \n";

    // Step 1: delete all direct-routes connected to this interface

    // ... from OSPF table
    for(uint32_t i = 0; i < ospfRouter->getRoutingTableEntryCount(); i++) {
        OspfRoutingTableEntry *ospfRoute = ospfRouter->getRoutingTableEntry(i);
        if(ospfRoute && ospfRoute->getInterface() == ie && ospfRoute->getNextHopAsGeneric().isUnspecified()) {
            EV_DEBUG << "removing route from OSPF routing table: " << ospfRoute << "\n";
            ospfRouter->deleteRoute(ospfRoute);
        }
    }
    // ... from Ipv4 table
    for(int32_t i = 0; i < rt->getNumRoutes(); i++) {
        Ipv4Route *route = rt->getRoute(i);
        if(route && route->getInterface() == ie && route->getNextHopAsGeneric().isUnspecified()) {
            EV_DEBUG << "removing route from Ipv4 routing table: " << route << "\n";
            rt->deleteRoute(route);
        }
    }

    // Step 2: find the OspfInterface associated with the ie
    OspfInterface *foundIntf = nullptr;
    for(auto &areaId : ospfRouter->getAreaIds()) {
        Area *area = ospfRouter->getAreaByID(areaId);
        if(area) {
            for(auto &ifIndex : area->getInterfaceIndices()) {
                OspfInterface *intf = area->getInterface(ifIndex);
                if(intf && intf->getIfIndex() == ie->getInterfaceId()) {
                    foundIntf = intf;
                    break;
                }
            }
            if(foundIntf)
                break;
        }
    }

    if(foundIntf) {
        // Step 3: Zero or more remote routes are not reachable any more.
        // Find all neighbors connected to this interface and reset them
        std::vector<Neighbor *> neighbors;
        int numNeighbors = foundIntf->getNeighborCount();
        for(int i = 0; i < numNeighbors; i++) {
            Neighbor *neighbor = foundIntf->getNeighbor(i);
            if(neighbor && neighbor->getState() != Neighbor::DOWN_STATE) {
                neighbor->processEvent(Neighbor::KILL_NEIGHBOR_NO_REBUILD);
                neighbors.push_back(neighbor);
            }
        }

        // Step 4: Rebuild the routing table
        for(auto &neighbor : neighbors)
            neighbor->processEvent(Neighbor::REBUILD);

        // Step 5: now we can take the interface down
        foundIntf->processEvent(OspfInterface::INTERFACE_DOWN);
    }
}

} // namespace ospf

} // namespace inet

