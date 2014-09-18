//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include <sstream>

#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"

#ifdef WITH_GENERIC
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"
#endif // ifdef WITH_GENERIC

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6

namespace inet {

Define_Module(InterfaceTable);

#define INTERFACEIDS_START    100

std::ostream& operator<<(std::ostream& os, const InterfaceEntry& e)
{
    os << e.info();
    return os;
};

InterfaceTable::InterfaceTable()
{
    host = NULL;
    tmpNumInterfaces = -1;
    tmpInterfaceList = NULL;
}

InterfaceTable::~InterfaceTable()
{
    for (int i = 0; i < (int)idToInterface.size(); i++)
        delete idToInterface[i];
    delete[] tmpInterfaceList;
}

void InterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // get a pointer to the host module
        host = getContainingNode(this);
        WATCH_PTRVECTOR(idToInterface);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        updateDisplayString();
    }
}

void InterfaceTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d interfaces", getNumInterfaces());
    getDisplayString().setTagArg("t", 0, buf);
}

void InterfaceTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void InterfaceTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    // nothing needed here at the moment
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);
}

//---

cModule *InterfaceTable::getHostModule()
{
    if (!host)
        host = getContainingNode(this);
    return host;
}

bool InterfaceTable::isLocalAddress(const L3Address& address) const
{
    return findInterfaceByAddress(address) != NULL;
}

InterfaceEntry *InterfaceTable::findInterfaceByAddress(const L3Address& address) const
{
    if (!address.isUnspecified()) {
        L3Address::AddressType addrType = address.getType();
        for (int i = 0; i < (int)idToInterface.size(); i++) {
            InterfaceEntry *ie = idToInterface[i];
            if (ie) {
#ifdef WITH_GENERIC
                if (ie->getGenericNetworkProtocolData() && ie->getGenericNetworkProtocolData()->getAddress() == address)
                    return ie;
#endif // ifdef WITH_GENERIC
                switch (addrType) {
#ifdef WITH_IPv4
                    case L3Address::IPv4:
                        if (ie->ipv4Data() && ie->ipv4Data()->getIPAddress() == address.toIPv4())
                            return ie;
                        break;
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
                    case L3Address::IPv6:
                        if (ie->ipv6Data() && ie->ipv6Data()->hasAddress(address.toIPv6()))
                            return ie;
                        break;
#endif // ifdef WITH_IPv6

                    case L3Address::MAC:
                        if (ie->getMacAddress() == address.toMAC())
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
    return NULL;
}

bool InterfaceTable::isNeighborAddress(const L3Address& address) const
{
    if (address.isUnspecified())
        return false;

    switch (address.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4:
            for (int i = 0; i < (int)idToInterface.size(); i++) {
                InterfaceEntry *ie = idToInterface[i];
                if (ie && ie->ipv4Data()) {
                    IPv4Address ipv4Addr = ie->ipv4Data()->getIPAddress();
                    IPv4Address netmask = ie->ipv4Data()->getNetmask();
                    if (IPv4Address::maskedAddrAreEqual(address.toIPv4(), ipv4Addr, netmask))
                        return address != ipv4Addr;
                }
            }
            break;

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            for (int i = 0; i < (int)idToInterface.size(); i++) {
                InterfaceEntry *ie = idToInterface[i];
                if (ie && ie->ipv6Data()) {
                    IPv6InterfaceData *ipv6Data = ie->ipv6Data();
                    for (int j = 0; j < ipv6Data->getNumAdvPrefixes(); j++) {
                        const IPv6InterfaceData::AdvPrefix& advPrefix = ipv6Data->getAdvPrefix(j);
                        if (address.toIPv6().matches(advPrefix.prefix, advPrefix.prefixLength))
                            return address != advPrefix.prefix;
                    }
                }
            }
            break;

#endif // ifdef WITH_IPv6
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

int InterfaceTable::getNumInterfaces()
{
    if (tmpNumInterfaces == -1) {
        // count non-NULL elements
        int n = 0;
        int maxId = idToInterface.size();
        for (int i = 0; i < maxId; i++)
            if (idToInterface[i])
                n++;

        tmpNumInterfaces = n;
    }

    return tmpNumInterfaces;
}

InterfaceEntry *InterfaceTable::getInterface(int pos)
{
    int n = getNumInterfaces();    // also fills tmpInterfaceList
    if (pos < 0 || pos >= n)
        throw cRuntimeError("getInterface(): interface index %d out of range 0..%d", pos, n - 1);

    if (!tmpInterfaceList) {
        // collect non-NULL elements into tmpInterfaceList[]
        tmpInterfaceList = new InterfaceEntry *[n];
        int k = 0;
        int maxId = idToInterface.size();
        for (int i = 0; i < maxId; i++)
            if (idToInterface[i])
                tmpInterfaceList[k++] = idToInterface[i];

    }

    return tmpInterfaceList[pos];
}

InterfaceEntry *InterfaceTable::getInterfaceById(int id)
{
    id -= INTERFACEIDS_START;
    return (id < 0 || id >= (int)idToInterface.size()) ? NULL : idToInterface[id];
}

int InterfaceTable::getBiggestInterfaceId()
{
    return INTERFACEIDS_START + idToInterface.size() - 1;
}

void InterfaceTable::addInterface(InterfaceEntry *entry)
{
    if (!host)
        throw cRuntimeError("InterfaceTable must precede all network interface modules in the node's NED definition");
    // check name is unique
    if (getInterfaceByName(entry->getName()) != NULL)
        throw cRuntimeError("addInterface(): interface '%s' already registered", entry->getName());

    // insert
    entry->setInterfaceId(INTERFACEIDS_START + idToInterface.size());
    entry->setInterfaceTable(this);
    idToInterface.push_back(entry);
    invalidateTmpInterfaceList();

    // fill in networkLayerGateIndex, nodeOutputGateId, nodeInputGateId
    discoverConnectingGates(entry);

    emit(NF_INTERFACE_CREATED, entry);
}

void InterfaceTable::discoverConnectingGates(InterfaceEntry *entry)
{
    cModule *ifmod = entry->getInterfaceModule();
    if (!ifmod)
        return; // virtual interface

    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    ASSERT(host != NULL);
    while (ifmod && ifmod->getParentModule() != host)
        ifmod = ifmod->getParentModule();
    if (!ifmod)
        throw cRuntimeError("addInterface(): specified module (%s) is not in this host/router '%s'", entry->getInterfaceModule()->getFullPath().c_str(), this->getFullPath().c_str());

    // ASSUMPTIONS:
    // 1. The NIC module (ifmod) may or may not be connected to a network layer module (e.g. IPv4NetworkLayer or MPLS)
    // 2. If it *is* connected to a network layer, the network layer module's gates must be called
    //    ifIn[] and ifOut[], and NIC must be connected to identical gate indices in both vectors.
    // 3. If the NIC module is not connected to another modules ifIn[] and ifOut[] gates, we assume
    //    that it is NOT connected to a network layer, and leave networkLayerGateIndex
    //    in InterfaceEntry unfilled.
    // 4. The NIC may or may not connect to gates of the containing host compound module.
    //

    // find gates connected to host / network layer
    cGate *nwlayerInGate = NULL, *nwlayerOutGate = NULL;    // ifIn[] and ifOut[] gates in the network layer
    for (GateIterator i(ifmod); !i.end(); i++) {
        cGate *g = i();
        if (!g)
            continue;

        // find the host/router's gates that internally connect to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->getOwnerModule() == host)
            entry->setNodeOutputGateId(g->getNextGate()->getId());
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->getOwnerModule() == host)
            entry->setNodeInputGateId(g->getPreviousGate()->getId());

        // find the gate index of networkLayer/networkLayer6/mpls that connects to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->isName("ifIn")) // connected to ifIn in networkLayer?
            nwlayerInGate = g->getNextGate();
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->isName("ifOut")) // connected to ifOut in networkLayer?
            nwlayerOutGate = g->getPreviousGate();
    }

    // consistency checks and setting networkLayerGateIndex:

    // note: we don't check nodeOutputGateId/nodeInputGateId, because wireless interfaces
    // are not connected to the host

    if (nwlayerInGate || nwlayerOutGate) {    // connected to a network layer (i.e. to another module's ifIn/ifOut gates)
        if (!nwlayerInGate || !nwlayerOutGate)
            throw cRuntimeError("addInterface(): interface module '%s' is connected only to an 'ifOut' or an 'ifIn' gate, must connect to either both or neither", ifmod->getFullPath().c_str());
        if (nwlayerInGate->getOwnerModule() != nwlayerOutGate->getOwnerModule())
            throw cRuntimeError("addInterface(): interface module '%s' is connected to 'ifOut' and 'ifIn' gates in different modules", ifmod->getFullPath().c_str());
        if (nwlayerInGate->getIndex() != nwlayerOutGate->getIndex()) // if both are scalar, that's OK too (index==0)
            throw cRuntimeError("addInterface(): gate index mismatch: interface module '%s' is connected to different indices in 'ifOut[']/'ifIn[]' gates of the network layer module", ifmod->getFullPath().c_str());
        entry->setNetworkLayerGateIndex(nwlayerInGate->getIndex());
    }
}

void InterfaceTable::deleteInterface(InterfaceEntry *entry)
{
    int id = entry->getInterfaceId();
    if (entry != getInterfaceById(id))
        throw cRuntimeError("deleteInterface(): interface '%s' not found in interface table", entry->getName());

    emit(NF_INTERFACE_DELETED, entry);    // actually, only going to be deleted

    idToInterface[id - INTERFACEIDS_START] = NULL;
    delete entry;
    invalidateTmpInterfaceList();
}

void InterfaceTable::invalidateTmpInterfaceList()
{
    tmpNumInterfaces = -1;
    delete[] tmpInterfaceList;
    tmpInterfaceList = NULL;
}

void InterfaceTable::interfaceChanged(simsignal_t signalID, const InterfaceEntryChangeDetails *details)
{
    Enter_Method_Silent();

    emit(signalID, const_cast<InterfaceEntryChangeDetails *>(details));

    if (ev.isGUI() && par("displayAddresses").boolValue())
        updateLinkDisplayString(details->getInterfaceEntry());
}

void InterfaceTable::updateLinkDisplayString(InterfaceEntry *entry)
{
    int outputGateId = entry->getNodeOutputGateId();
    if (outputGateId != -1) {
        ASSERT(host != NULL);
        cGate *outputGate = host->gate(outputGateId);
        if (!outputGate->getChannel())
            return;
        cDisplayString& displayString = outputGate->getDisplayString();
        char buf[128];
#ifdef WITH_IPv4
        if (entry->ipv4Data()) {
            sprintf(buf, "%s\n%s/%d", entry->getFullName(), entry->ipv4Data()->getIPAddress().str().c_str(), entry->ipv4Data()->getNetmask().getNetmaskLength());
            displayString.setTagArg("t", 0, buf);
            displayString.setTagArg("t", 1, "l");
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        if (entry->ipv6Data() && entry->ipv6Data()->getNumAddresses() > 0) {
            sprintf(buf, "%s\n%s", entry->getFullName(), entry->ipv6Data()->getPreferredAddress().str().c_str());
            displayString.setTagArg("t", 0, buf);
            displayString.setTagArg("t", 1, "l");
        }
#endif // ifdef WITH_IPv6
    }
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeOutputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeOutputGateId() == id)
            return idToInterface[i];

    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNodeInputGateId(int id)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->getNodeInputGateId() == id)
            return idToInterface[i];

    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByNetworkLayerGateIndex(int index)
{
    // linear search is OK because normally we have don't have many interfaces and this func is rarely called
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->getNetworkLayerGateIndex() == index)
            return idToInterface[i];

    return NULL;
}

InterfaceEntry *InterfaceTable::getInterfaceByInterfaceModule(cModule *ifmod)
{
    // ifmod is something like "host.eth[1].mac"; climb up to find "host.eth[1]" from it
    ASSERT(host != NULL);
    cModule *_ifmod = ifmod;
    while (ifmod && ifmod->getParentModule() != host)
        ifmod = ifmod->getParentModule();
    if (!ifmod)
        throw cRuntimeError("addInterface(): specified module (%s) is not in this host/router '%s'", _ifmod->getFullPath().c_str(), this->getFullPath().c_str());

    int nodeInputGateId = -1, nodeOutputGateId = -1;
    for (GateIterator i(ifmod); !i.end(); i++) {
        cGate *g = i();
        if (!g)
            continue;

        // find the host/router's gates that internally connect to this interface
        if (g->getType() == cGate::OUTPUT && g->getNextGate() && g->getNextGate()->getOwnerModule() == host)
            nodeOutputGateId = g->getNextGate()->getId();
        if (g->getType() == cGate::INPUT && g->getPreviousGate() && g->getPreviousGate()->getOwnerModule() == host)
            nodeInputGateId = g->getPreviousGate()->getId();
    }

    InterfaceEntry *ie = NULL;
    if (nodeInputGateId >= 0)
        ie = getInterfaceByNodeInputGateId(nodeInputGateId);
    if (!ie && nodeOutputGateId >= 0)
        ie = getInterfaceByNodeOutputGateId(nodeOutputGateId);

    ASSERT(!ie || (ie->getNodeInputGateId() == nodeInputGateId && ie->getNodeOutputGateId() == nodeOutputGateId));
    return ie;
}

InterfaceEntry *InterfaceTable::getInterfaceByName(const char *name)
{
    Enter_Method_Silent();
    if (!name)
        return NULL;
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && !strcmp(name, idToInterface[i]->getName()))
            return idToInterface[i];

    return NULL;
}

InterfaceEntry *InterfaceTable::getFirstLoopbackInterface()
{
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->isLoopback())
            return idToInterface[i];

    return NULL;
}

InterfaceEntry *InterfaceTable::getFirstMulticastInterface()
{
    Enter_Method_Silent();
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i] && idToInterface[i]->isMulticast() && !idToInterface[i]->isLoopback())
            return idToInterface[i];

    return NULL;
}

bool InterfaceTable::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_LINK_LAYER)
            resetInterfaces();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            resetInterfaces();
    }
    return true;
}

void InterfaceTable::resetInterfaces()
{
    int n = idToInterface.size();
    for (int i = 0; i < n; i++)
        if (idToInterface[i])
            idToInterface[i]->resetInterface();

}

MulticastGroupList InterfaceTable::collectMulticastGroups()
{
    MulticastGroupList mglist;
    for (int i = 0; i < getNumInterfaces(); ++i) {
        InterfaceEntry *ie = getInterface(i);
        int interfaceId = ie->getInterfaceId();
#ifdef WITH_IPv4
        if (ie->ipv4Data()) {
            int numOfMulticastGroups = ie->ipv4Data()->getNumOfJoinedMulticastGroups();
            for (int j = 0; j < numOfMulticastGroups; ++j) {
                mglist.push_back(MulticastGroup(ie->ipv4Data()->getJoinedMulticastGroup(j), interfaceId));
            }
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        // TODO
#endif // ifdef WITH_IPv6
    }
    return mglist;
}

} // namespace inet

