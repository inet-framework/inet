/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
 * Copyright (C) 2014 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/networklayer/arp/ipv4/GlobalARP.h"

#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

static std::ostream& operator<<(std::ostream& out, const GlobalARP::ARPCacheEntry& e)
{
    out << "MAC:" << e.macAddress << "  age:" << floor(simTime() - e.lastUpdate) << "s";
    return out;
}

GlobalARP::ARPCache GlobalARP::globalArpCache;
int GlobalARP::globalArpCacheRefCnt = 0;

Define_Module(GlobalARP);

GlobalARP::GlobalARP()
{
    if (++globalArpCacheRefCnt == 1) {
        if (!globalArpCache.empty())
            throw cRuntimeError("Global ARP cache not empty, model error in previous run?");
    }

    ift = NULL;
    rt = NULL;
}

void GlobalARP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRMAP(globalArpCache);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {    // IP addresses should be available
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);

        isUp = isNodeUp();

        // register our addresses in the global cache
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isLoopback())
                continue;
            if (!ie->ipv4Data())
                continue;
            IPv4Address nextHopAddr = ie->ipv4Data()->getIPAddress();
            if (nextHopAddr.isUnspecified())
                continue; // if the address is not defined it isn't included in the global cache
            // check if the entry exist
            ARPCache::iterator it = globalArpCache.find(nextHopAddr);
            if (it != globalArpCache.end())
                continue;
            ARPCacheEntry *entry = new ARPCacheEntry();
            entry->owner = this;
            entry->ie = ie;
            entry->macAddress = ie->getMacAddress();

            ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(nextHopAddr, entry));
            ASSERT(where->second == entry);
            entry->myIter = where;    // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        }
        cModule *host = findContainingNode(this);
        if (host != NULL)
            host->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
}

void GlobalARP::finish()
{
}

GlobalARP::~GlobalARP()
{
    --globalArpCacheRefCnt;
    // delete my entries from the globalArpCache
    for (ARPCache::iterator it = globalArpCache.begin(); it != globalArpCache.end(); ) {
        if (it->second->owner == this) {
            ARPCache::iterator cur = it++;
            delete cur->second;
            globalArpCache.erase(cur);
        }
        else
            ++it;
    }
}

void GlobalARP::handleMessage(cMessage *msg)
{
    if (!isUp) {
        handleMessageWhenDown(msg);
        return;
    }

    if (msg->isSelfMessage())
        processSelfMessage(msg);
    else
        processARPPacket(check_and_cast<ARPPacket *>(msg));

    if (ev.isGUI())
        updateDisplayString();
}

void GlobalARP::processSelfMessage(cMessage *msg)
{
    throw cRuntimeError("Model error: unexpected self message");
}

void GlobalARP::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
    EV_ERROR << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool GlobalARP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_NETWORK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_NETWORK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }
    return true;
}

void GlobalARP::start()
{
    isUp = true;
}

void GlobalARP::stop()
{
    isUp = false;
}

bool GlobalARP::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void GlobalARP::updateDisplayString()
{
}

void GlobalARP::processARPPacket(ARPPacket *arp)
{
    EV << "ARP packet " << arp << " arrived, dropped\n";
    delete arp;
}

MACAddress GlobalARP::resolveL3Address(const L3Address& address, const InterfaceEntry *ie)
{
    Enter_Method_Silent();

    IPv4Address addr = address.toIPv4();
    ARPCache::const_iterator it = globalArpCache.find(addr);
    if (it != globalArpCache.end())
        return it->second->macAddress;

    throw cRuntimeError("GlobalARP does not support dynamic address resolution");
    return MACAddress::UNSPECIFIED_ADDRESS;
}

L3Address GlobalARP::getL3AddressFor(const MACAddress& macAddr) const
{
    Enter_Method_Silent();

    if (macAddr.isUnspecified())
        return IPv4Address::UNSPECIFIED_ADDRESS;

    for (ARPCache::const_iterator it = globalArpCache.begin(); it != globalArpCache.end(); it++)
        if (it->second->macAddress == macAddr && it->first.getType() == L3Address::IPv4)
            return it->first.toIPv4();


    return IPv4Address::UNSPECIFIED_ADDRESS;
}

void GlobalARP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    // host associated. Link is up. Change the state to init.
    if (signalID == NF_INTERFACE_IPv4CONFIG_CHANGED) {
        const InterfaceEntryChangeDetails *iecd = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        InterfaceEntry *ie = iecd->getInterfaceEntry();
        // rebuild the arp cache
        if (ie->isLoopback())
            return;
        ARPCache::iterator it;
        for (it = globalArpCache.begin(); it != globalArpCache.end(); ++it) {
            if (it->second->ie == ie)
                break;
        }
        ARPCacheEntry *entry = NULL;
        if (it == globalArpCache.end()) {
            if (!ie->ipv4Data() || ie->ipv4Data()->getIPAddress().isUnspecified())
                return; // if the address is not defined it isn't included in the global cache
            entry = new ARPCacheEntry();
            entry->owner = this;
            entry->ie = ie;
        }
        else {
            // actualize
            entry = it->second;
            ASSERT(entry->owner == this);
            globalArpCache.erase(it);
            if (!ie->ipv4Data() || ie->ipv4Data()->getIPAddress().isUnspecified()) {
                delete entry;
                return;    // if the address is not defined it isn't included in the global cache
            }
        }
        entry->macAddress = ie->getMacAddress();
        IPv4Address ipAddr = ie->ipv4Data()->getIPAddress();
        ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(ipAddr, entry));
        ASSERT(where->second == entry);
        entry->myIter = where;    // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
    }
}

} // namespace inet

