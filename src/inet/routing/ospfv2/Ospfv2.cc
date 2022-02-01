//
// Copyright (C) 2006 Andras Babos and Andras Varga
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/Ospfv2.h"

#include <memory.h>
#include <stdlib.h>

#include <map>
#include <string>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/routing/ospfv2/Ospfv2ConfigReader.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"

namespace inet {
namespace ospfv2 {

Define_Module(Ospfv2);

Ospfv2::Ospfv2()
{
}

Ospfv2::~Ospfv2()
{
    cancelAndDelete(startupTimer);
    delete ospfRouter;
}

void Ospfv2::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        startupTimer = new cMessage("OSPF-startup");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) { // interfaces and static routes are already initialized
        registerProtocol(Protocol::ospf, gate("ipOut"), gate("ipIn"));
    }
}

void Ospfv2::handleMessageWhenUp(cMessage *msg)
{
    if (msg == startupTimer) {
        createOspfRouter();
        subscribe();
    }
    else
        ospfRouter->getMessageHandler()->messageReceived(msg);

}

void Ospfv2::createOspfRouter()
{
    ospfRouter = new Router(this, ift, rt);

    // read the OSPF AS configuration
    cXMLElement *ospfConfig = par("ospfConfig");
    Ospfv2ConfigReader configReader(this, ift);
    if (!configReader.loadConfigFromXML(ospfConfig, ospfRouter))
        throw cRuntimeError("Error reading AS configuration from %s", ospfConfig->getSourceLocation());

    ospfRouter->addWatches();
}

void Ospfv2::subscribe()
{
    host->subscribe(interfaceCreatedSignal, this);
    host->subscribe(interfaceDeletedSignal, this);
    host->subscribe(interfaceStateChangedSignal, this);
}

void Ospfv2::unsubscribe()
{
    host->unsubscribe(interfaceCreatedSignal, this);
    host->unsubscribe(interfaceDeletedSignal, this);
    host->unsubscribe(interfaceStateChangedSignal, this);
}

/**
 * Listen on interface changes and update private data structures.
 */
void Ospfv2::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    const NetworkInterface *ie;
    const NetworkInterfaceChangeDetails *change;

    if (signalID == interfaceCreatedSignal) {
        // configure interface for RIP
        ie = check_and_cast<const NetworkInterface *>(obj);
        if (ie->isMulticast() && !ie->isLoopback()) {
            // TODO
        }
    }
    else if (signalID == interfaceDeletedSignal) {
        ie = check_and_cast<const NetworkInterface *>(obj);
        // TODO
    }
    else if (signalID == interfaceStateChangedSignal) {
        change = check_and_cast<const NetworkInterfaceChangeDetails *>(obj);
        auto fieldId = change->getFieldId();
        if (fieldId == NetworkInterface::F_STATE || fieldId == NetworkInterface::F_CARRIER) {
            ie = change->getNetworkInterface();
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

void Ospfv2::handleStartOperation(LifecycleOperation *operation)
{
    ASSERT(ospfRouter == nullptr);
    simtime_t startupTime = par("startupTime");
    if (startupTime <= simTime()) {
        createOspfRouter();
        subscribe();
    }
    else
        scheduleAfter(startupTime, startupTimer);
}

void Ospfv2::handleStopOperation(LifecycleOperation *operation)
{
    ASSERT(ospfRouter);
    delete ospfRouter;
    cancelEvent(startupTimer);
    ospfRouter = nullptr;
    unsubscribe();
}

void Ospfv2::handleCrashOperation(LifecycleOperation *operation)
{
    ASSERT(ospfRouter);
    delete ospfRouter;
    cancelEvent(startupTimer);
    ospfRouter = nullptr;
    unsubscribe();
}

void Ospfv2::insertExternalRoute(int ifIndex, const Ipv4AddressRange& netAddr)
{
    Enter_Method("insertExternalRoute");
    Ospfv2AsExternalLsaContents newExternalContents;
    newExternalContents.setExternalTOSInfoArraySize(1);
    const Ipv4Address netmask = netAddr.mask;
    newExternalContents.setNetworkMask(netmask);
    auto& tosInfo = newExternalContents.getExternalTOSInfoForUpdate(0);
    tosInfo.E_ExternalMetricType = false;
    tosInfo.tos = 0;
    tosInfo.externalRouteTag = OSPFv2_EXTERNAL_ROUTES_LEARNED_BY_BGP;
//    tosInfo.forwardingAddress = ;
    tosInfo.routeCost = OSPFv2_BGP_DEFAULT_COST;

    ospfRouter->updateExternalRoute(netAddr.address, newExternalContents, ifIndex);
}

int Ospfv2::checkExternalRoute(const Ipv4Address& route)
{
    Enter_Method("checkExternalRoute");
    for (uint32_t i = 0; i < ospfRouter->getASExternalLSACount(); i++) {
        AsExternalLsa *externalLSA = ospfRouter->getASExternalLSA(i);
        Ipv4Address externalAddr = externalLSA->getHeader().getLinkStateID();
        if (externalAddr == route) { // FIXME was this meant???
            if (externalLSA->getContents().getExternalTOSInfo(0).E_ExternalMetricType)
                return 2;
            else
                return 1;
        }
    }
    return 0;
}

void Ospfv2::handleInterfaceDown(const NetworkInterface *ie)
{
    EV_DEBUG << "interface " << ie->getInterfaceId() << " went down. \n";

    // Step 1: delete all direct-routes connected to this interface

    // ... from OSPF table
    for (uint32_t i = 0; i < ospfRouter->getRoutingTableEntryCount(); i++) {
        Ospfv2RoutingTableEntry *ospfRoute = ospfRouter->getRoutingTableEntry(i);
        if (ospfRoute && ospfRoute->getInterface() == ie && ospfRoute->getNextHopAsGeneric().isUnspecified()) {
            EV_DEBUG << "removing route from OSPF routing table: " << ospfRoute << "\n";
            ospfRouter->deleteRoute(ospfRoute);
            i--;
        }
    }
    // ... from Ipv4 table
    for (int32_t i = 0; i < rt->getNumRoutes(); i++) {
        Ipv4Route *route = rt->getRoute(i);
        if (route && route->getInterface() == ie && route->getNextHopAsGeneric().isUnspecified()) {
            EV_DEBUG << "removing route from Ipv4 routing table: " << route << "\n";
            rt->deleteRoute(route);
            i--;
        }
    }

    // Step 2: find the Ospfv2Interface associated with the ie and take it down
    Ospfv2Interface *foundIntf = nullptr;
    for (auto& areaId : ospfRouter->getAreaIds()) {
        Ospfv2Area *area = ospfRouter->getAreaByID(areaId);
        if (area) {
            for (auto& ifIndex : area->getInterfaceIndices()) {
                Ospfv2Interface *intf = area->getInterface(ifIndex);
                if (intf && intf->getIfIndex() == ie->getInterfaceId()) {
                    foundIntf = intf;
                    break;
                }
            }
            if (foundIntf) {
                foundIntf->processEvent(Ospfv2Interface::INTERFACE_DOWN);
                break;
            }
        }
    }
}

} // namespace ospfv2
} // namespace inet

