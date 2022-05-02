//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/InterfaceTable.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/NodeStatus.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef INET_WITH_IPv6

#ifdef INET_WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#endif // ifdef INET_WITH_NEXTHOP

namespace inet {

Define_Module(InterfaceTable);

#define INTERFACEIDS_START    100

std::ostream& operator<<(std::ostream& os, const NetworkInterface& e)
{
    os << e.str();
    return os;
};

InterfaceTable::InterfaceTable()
{
    host = nullptr;
    tmpNumInterfaces = -1;
    tmpInterfaceList = nullptr;
}

InterfaceTable::~InterfaceTable()
{
    delete[] tmpInterfaceList;
}

void InterfaceTable::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the host module
        host = getContainingNode(this);
        WATCH_PTRVECTOR(idToInterface);
    }
}

void InterfaceTable::refreshDisplay() const
{
    OperationalBase::refreshDisplay();

    char buf[80];
    sprintf(buf, "%d interfaces", getNumInterfaces());
    getDisplayString().setTagArg("t", 0, buf);

    if (par("displayAddresses")) {
        for (auto& elem : idToInterface) {
            NetworkInterface *ie = elem;
            if (ie)
                updateLinkDisplayString(ie);
        }
    }
}

void InterfaceTable::handleMessageWhenUp(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void InterfaceTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    // nothing needed here at the moment
    printSignalBanner(signalID, obj, details);
}

// ---

cModule *InterfaceTable::getHostModule() const
{
    ASSERT(host != nullptr);
    return host;
}

bool InterfaceTable::isLocalAddress(const L3Address& address) const
{
    return findInterfaceByAddress(address) != nullptr;
}

NetworkInterface *InterfaceTable::findInterfaceByAddress(const L3Address& address) const
{
    if (!address.isUnspecified()) {
        L3Address::AddressType addrType = address.getType();
        for (auto& elem : idToInterface) {
            NetworkInterface *ie = elem;
            if (ie) {
#ifdef INET_WITH_NEXTHOP
                if (auto nextHopData = ie->findProtocolData<NextHopInterfaceData>())
                    if (nextHopData->getAddress() == address)
                        return ie;
#endif // ifdef INET_WITH_NEXTHOP
                switch (addrType) {
#ifdef INET_WITH_IPv4
                    case L3Address::IPv4:
                        if (auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>())
                            if (ipv4Data->getIPAddress() == address.toIpv4())
                                return ie;
                        break;
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
                    case L3Address::IPv6:
                        if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>())
                            if (ipv6Data->hasAddress(address.toIpv6()))
                                return ie;
                        break;
#endif // ifdef INET_WITH_IPv6

                    case L3Address::MAC:
                        if (ie->getMacAddress() == address.toMac())
                            return ie;
                        break;

                    case L3Address::MODULEID:
                        if (ie->getModuleIdAddress() == address.toModuleId())
                            return ie;
                        break;

                    case L3Address::MODULEPATH:
                        if (ie->getModulePathAddress() == address.toModulePath())
                            return ie;
                        break;

                    default:
                        throw cRuntimeError("Unknown address type");
                        break;
                }
            }
        }
    }
    return nullptr;
}

bool InterfaceTable::isNeighborAddress(const L3Address& address) const
{
    if (address.isUnspecified())
        return false;

    switch (address.getType()) {
#ifdef INET_WITH_IPv4
        case L3Address::IPv4:
            for (auto& elem : idToInterface) {
                NetworkInterface *ie = elem;
                if (ie) {
                    if (auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>()) {
                        Ipv4Address ipv4Addr = ipv4Data->getIPAddress();
                        Ipv4Address netmask = ipv4Data->getNetmask();
                        if (Ipv4Address::maskedAddrAreEqual(address.toIpv4(), ipv4Addr, netmask))
                            return address != ipv4Addr;
                    }
                }
            }
            break;

#endif // ifdef INET_WITH_IPv4
#ifdef INET_WITH_IPv6
        case L3Address::IPv6:
            for (auto& elem : idToInterface) {
                NetworkInterface *ie = elem;
                if (ie) {
                    if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>()) {
                        for (int j = 0; j < ipv6Data->getNumAdvPrefixes(); j++) {
                            const Ipv6InterfaceData::AdvPrefix& advPrefix = ipv6Data->getAdvPrefix(j);
                            if (address.toIpv6().matches(advPrefix.prefix, advPrefix.prefixLength))
                                return address != advPrefix.prefix;
                        }
                    }
                }
            }
            break;

#endif // ifdef INET_WITH_IPv6
        case L3Address::MAC:
        case L3Address::MODULEPATH:
        case L3Address::MODULEID:
            // TODO
            break;

        default:
            throw cRuntimeError("Unknown address type");
    }
    return false;
}

int InterfaceTable::getNumInterfaces() const
{
    if (tmpNumInterfaces == -1) {
        // count non-nullptr elements
        int n = 0;
        int maxId = idToInterface.size();
        for (int i = 0; i < maxId; i++)
            if (idToInterface[i])
                n++;

        tmpNumInterfaces = n;
    }

    return tmpNumInterfaces;
}

NetworkInterface *InterfaceTable::getInterface(int pos) const
{
    int n = getNumInterfaces(); // also fills tmpInterfaceList
    if (pos < 0 || pos >= n)
        throw cRuntimeError("getInterface(): interface index %d out of range 0..%d", pos, n - 1);

    if (!tmpInterfaceList) {
        // collect non-nullptr elements into tmpInterfaceList[]
        tmpInterfaceList = new NetworkInterface *[n];
        int k = 0;
        int maxId = idToInterface.size();
        for (int i = 0; i < maxId; i++)
            if (idToInterface[i])
                tmpInterfaceList[k++] = idToInterface[i];

    }

    return tmpInterfaceList[pos];
}

NetworkInterface *InterfaceTable::getInterfaceById(int id) const
{
    NetworkInterface *ie = findInterfaceById(id);
    if (ie == nullptr)
        throw cRuntimeError("getInterfaceById(): no interface with ID=%d", id);
    return ie;
}

NetworkInterface *InterfaceTable::findInterfaceById(int id) const
{
    id -= INTERFACEIDS_START;
    return (id < 0 || id >= (int)idToInterface.size()) ? nullptr : idToInterface[id];
}

int InterfaceTable::getBiggestInterfaceId() const
{
    return INTERFACEIDS_START + idToInterface.size() - 1;
}

void InterfaceTable::addInterface(NetworkInterface *entry)
{
    if (!host)
        throw cRuntimeError("InterfaceTable must precede all network interface modules in the node's NED definition");
    // check name is unique
    if (findInterfaceByName(entry->getInterfaceName()) != nullptr)
        throw cRuntimeError("addInterface(): interface '%s' already registered", entry->getInterfaceName());

    // insert
    entry->setInterfaceId(INTERFACEIDS_START + idToInterface.size());
    idToInterface.push_back(entry);
    invalidateTmpInterfaceList();

    // fill in networkLayerGateIndex, nodeOutputGateId, nodeInputGateId
    discoverConnectingGates(entry);

    emit(interfaceCreatedSignal, entry);
}

void InterfaceTable::discoverConnectingGates(NetworkInterface *entry)
{
    cModule *ifmod = entry;
    if (!ifmod)
        return; // virtual interface

    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    ASSERT(host != nullptr);
    while (ifmod && ifmod->getParentModule() != host)
        ifmod = ifmod->getParentModule();
    if (!ifmod)
        throw cRuntimeError("addInterface(): specified module (%s) is not in this host/router '%s'", entry->getInterfaceFullPath().c_str(), this->getFullPath().c_str());

    // ASSUMPTIONS:
    // 1. The NIC module (ifmod) may or may not be connected to a network layer module (e.g. Ipv4NetworkLayer or Mpls)
    // 2. If it *is* connected to a network layer, the network layer module's gates must be called
    //    ifIn[] and ifOut[], and NIC must be connected to identical gate indices in both vectors.
    // 3. If the NIC module is not connected to another modules ifIn[] and ifOut[] gates, we assume
    //    that it is NOT connected to a network layer, and leave networkLayerGateIndex
    //    in NetworkInterface unfilled.
    // 4. The NIC may or may not connect to gates of the containing host compound module.
    //

    // find gates connected to host / network layer
    cGate *nwlayerInGate = nullptr, *nwlayerOutGate = nullptr; // ifIn[] and ifOut[] gates in the network layer
    for (GateIterator i(ifmod); !i.end(); i++) {
        cGate *g = *i;
        if (!g)
            continue;

        // find the host/router's gates that internally connect to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->getOwnerModule() == host)
            entry->setNodeOutputGateId(g->getNextGate()->getId());
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->getOwnerModule() == host)
            entry->setNodeInputGateId(g->getPreviousGate()->getId());

        // TODO revise next code:
        // find the gate index of networkLayer/networkLayer6/mpls that connects to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->isName("ifIn")) // connected to ifIn in networkLayer?
            nwlayerInGate = g->getNextGate();
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->isName("ifOut")) // connected to ifOut in networkLayer?
            nwlayerOutGate = g->getPreviousGate();
    }

    // consistency checks and setting networkLayerGateIndex:

    // note: we don't check nodeOutputGateId/nodeInputGateId, because wireless interfaces
    // are not connected to the host

    // TODO revise next code:
    if (nwlayerInGate || nwlayerOutGate) { // connected to a network layer (i.e. to another module's ifIn/ifOut gates)
        if (!nwlayerInGate || !nwlayerOutGate)
            throw cRuntimeError("addInterface(): interface module '%s' is connected only to an 'ifOut' or an 'ifIn' gate, must connect to either both or neither", ifmod->getFullPath().c_str());
        if (nwlayerInGate->getOwnerModule() != nwlayerOutGate->getOwnerModule())
            throw cRuntimeError("addInterface(): interface module '%s' is connected to 'ifOut' and 'ifIn' gates in different modules", ifmod->getFullPath().c_str());
        if (nwlayerInGate->getIndex() != nwlayerOutGate->getIndex()) // if both are scalar, that's OK too (index==0)
            throw cRuntimeError("addInterface(): gate index mismatch: interface module '%s' is connected to different indices in 'ifOut[']/'ifIn[]' gates of the network layer module", ifmod->getFullPath().c_str());
//        entry->setNetworkLayerGateIndex(nwlayerInGate->getIndex());
    }
}

void InterfaceTable::deleteInterface(NetworkInterface *entry)
{
    int id = entry->getInterfaceId();
    if (entry != getInterfaceById(id))
        throw cRuntimeError("deleteInterface(): interface '%s' not found in interface table", entry->getInterfaceName());

    emit(interfaceDeletedSignal, entry); // actually, only going to be deleted

    idToInterface[id - INTERFACEIDS_START] = nullptr;
    entry->deleteModule();
    invalidateTmpInterfaceList();
}

void InterfaceTable::invalidateTmpInterfaceList()
{
    tmpNumInterfaces = -1;
    delete[] tmpInterfaceList;
    tmpInterfaceList = nullptr;
}

void InterfaceTable::interfaceChanged(simsignal_t signalID, const NetworkInterfaceChangeDetails *details)
{
    Enter_Method("interfaceChanged");
    emit(signalID, const_cast<NetworkInterfaceChangeDetails *>(details));
}

void InterfaceTable::updateLinkDisplayString(NetworkInterface *entry) const
{
    int outputGateId = entry->getNodeOutputGateId();
    if (outputGateId != -1) {
        ASSERT(host != nullptr);
        cGate *outputGate = host->gate(outputGateId);
        if (!outputGate->getChannel())
            return;
        cDisplayString& displayString = outputGate->getDisplayString();
        std::stringstream buf;
        buf << entry->getFullName() << "\n";
#ifdef INET_WITH_IPv4
        auto ipv4Data = entry->findProtocolData<Ipv4InterfaceData>();
        if (ipv4Data && !(ipv4Data->getIPAddress().isUnspecified())) {
            buf << ipv4Data->getIPAddress().str() << "/" << ipv4Data->getNetmask().getNetmaskLength() << "\n";
        }
#endif // ifdef INET_WITH_IPv4
#ifdef INET_WITH_IPv6
        auto ipv6Data = entry->findProtocolData<Ipv6InterfaceData>();
        if (ipv6Data && ipv6Data->getNumAddresses() > 0) {
            for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
                if (ipv6Data->getAddress(i).isSolicitedNodeMulticastAddress()
                        //|| (ipv6Data->getAddress(i).isLinkLocal() && ipv6Data->getAddress(i).)
                        || ipv6Data->getAddress(i).isMulticast()) continue;
                buf << ipv6Data->getAddress(i).str() << "/64" << "\n";
            }
        }
#endif // ifdef INET_WITH_IPv6
        displayString.setTagArg("t", 0, buf.str().c_str());
        displayString.setTagArg("t", 1, "l");
    }
}

NetworkInterface *InterfaceTable::findInterfaceByNodeOutputGateId(int id) const
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method("findInterfaceByNodeOutputGateId");
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeOutputGateId() == id)
            return idToInterface[i];

    return nullptr;
}

NetworkInterface *InterfaceTable::findInterfaceByNodeInputGateId(int id) const
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method("findInterfaceByNodeInputGateId");
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeInputGateId() == id)
            return idToInterface[i];

    return nullptr;
}

NetworkInterface *InterfaceTable::findInterfaceByInterfaceModule(cModule *ifmod) const
{
    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    ASSERT(host != nullptr);
    cModule *_ifmod = ifmod;
    while (ifmod && ifmod->getParentModule() != host)
        ifmod = ifmod->getParentModule();
    if (!ifmod)
        throw cRuntimeError("Specified module (%s) is not in this host/router '%s'", _ifmod->getFullPath().c_str(), this->getFullPath().c_str());

    int nodeInputGateId = -1, nodeOutputGateId = -1;
    for (GateIterator i(ifmod); !i.end(); i++) {
        cGate *g = *i;
        if (!g)
            continue;

        // find the host/router's gates that internally connect to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->getOwnerModule() == host)
            nodeOutputGateId = g->getNextGate()->getId();
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->getOwnerModule() == host)
            nodeInputGateId = g->getPreviousGate()->getId();
    }

    NetworkInterface *ie = nullptr;
    if (nodeInputGateId >= 0)
        ie = findInterfaceByNodeInputGateId(nodeInputGateId);
    if (!ie && nodeOutputGateId >= 0)
        ie = findInterfaceByNodeOutputGateId(nodeOutputGateId);

    ASSERT(!ie || (ie->getNodeInputGateId() == nodeInputGateId && ie->getNodeOutputGateId() == nodeOutputGateId));
    return ie;
}

NetworkInterface *InterfaceTable::findInterfaceByName(const char *name) const
{
    Enter_Method("findInterfaceByName");
    if (!name)
        return nullptr;
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && !strcmp(name, idToInterface[i]->getInterfaceName()))
            return idToInterface[i];

    return nullptr;
}

NetworkInterface *InterfaceTable::findFirstLoopbackInterface() const
{
    Enter_Method("findFirstLoopbackInterface");
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->isLoopback())
            return idToInterface[i];

    return nullptr;
}

NetworkInterface *InterfaceTable::findFirstNonLoopbackInterface() const
{
    Enter_Method("findFirstNonLoopbackInterface");
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && !idToInterface[i]->isLoopback())
            return idToInterface[i];

    return nullptr;
}

NetworkInterface *InterfaceTable::findFirstMulticastInterface() const
{
    Enter_Method("findFirstMulticastInterface");
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->isMulticast() && !idToInterface[i]->isLoopback())
            return idToInterface[i];

    return nullptr;
}

void InterfaceTable::handleStartOperation(LifecycleOperation *operation)
{
}

void InterfaceTable::handleStopOperation(LifecycleOperation *operation)
{
    resetInterfaces();
}

void InterfaceTable::handleCrashOperation(LifecycleOperation *operation)
{
    resetInterfaces();
}

void InterfaceTable::resetInterfaces()
{
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i])
            idToInterface[i]->resetInterface();
}

MulticastGroupList InterfaceTable::collectMulticastGroups() const
{
    MulticastGroupList mglist;
    for (int i = 0; i < getNumInterfaces(); ++i) {
#ifdef INET_WITH_IPv4
        NetworkInterface *ie = getInterface(i);
        int interfaceId = ie->getInterfaceId();
        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        if (ipv4Data) {
            int numOfMulticastGroups = ipv4Data->getNumOfJoinedMulticastGroups();
            for (int j = 0; j < numOfMulticastGroups; ++j) {
                mglist.push_back(MulticastGroup(ipv4Data->getJoinedMulticastGroup(j), interfaceId));
            }
        }
#endif // ifdef INET_WITH_IPv4
#ifdef INET_WITH_IPv6
        // TODO
#endif // ifdef INET_WITH_IPv6
    }
    return mglist;
}

} // namespace inet

