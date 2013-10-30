/*
 * Copyright (C) 2004 Andras Varga
 * Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
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


#include "ARP.h"

#include "Ieee802Ctrl_m.h"
#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"
#include "IPv4InterfaceData.h"
#include "IIPv4RoutingTable.h"
#include "IPv4RoutingTableAccess.h"
#include "ARPPacket_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"


simsignal_t ARP::sentReqSignal = registerSignal("sentReq");
simsignal_t ARP::sentReplySignal = registerSignal("sentReply");
simsignal_t ARP::initiatedResolutionSignal = registerSignal("initiatedResolution");
simsignal_t ARP::failedResolutionSignal = registerSignal("failedResolution");


static std::ostream& operator<<(std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

static std::ostream& operator<<(std::ostream& out, const ARP::ARPCacheEntry& e)
{
    if (e.pending)
        out << "pending (" << e.numRetries << " retries)";
    else
        out << "MAC:" << e.macAddress << "  age:" << floor(simTime()-e.lastUpdate) << "s";
    return out;
}

ARP::ARPCache ARP::globalArpCache;
int ARP::globalArpCacheRefCnt = 0;

Define_Module(ARP);

ARP::ARP()
{
    if (++globalArpCacheRefCnt == 1)
    {
        if (!globalArpCache.empty())
            throw cRuntimeError("Global ARP cache not empty, model error in previous run?");
    }

    ift = NULL;
    rt = NULL;
}

void ARP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        retryTimeout = par("retryTimeout");
        retryCount = par("retryCount");
        cacheTimeout = par("cacheTimeout");
        respondToProxyARP = par("respondToProxyARP");
        globalARP = par("globalARP");

        pendingQueue.setName("pendingQueue");

        netwOutGate = gate("netwOut");

        // init statistics
        numRequestsSent = numRepliesSent = 0;
        numResolutions = numFailedResolutions = 0;
        WATCH(numRequestsSent);
        WATCH(numRepliesSent);
        WATCH(numResolutions);
        WATCH(numFailedResolutions);

        WATCH_PTRMAP(arpCache);
        WATCH_PTRMAP(globalArpCache);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3)  // IP addresses should be available
    {
        ift = InterfaceTableAccess().get();
        rt = check_and_cast<IIPv4RoutingTable *>(getModuleByPath(par("routingTableModule")));

        isUp = isNodeUp();

        // register our addresses in the global cache (even if globalARP is locally turned off)
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
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
            if (it!=globalArpCache.end())
                continue;
            ARPCacheEntry *entry = new ARPCacheEntry();
            entry->owner = this;
            entry->ie = ie;
            entry->pending = false;
            entry->timer = NULL;
            entry->numRetries = 0;
            entry->macAddress = ie->getMacAddress();

            ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(nextHopAddr, entry));
            ASSERT(where->second == entry);
            entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        }
        cModule *host = findContainingNode(this);
        if (host != NULL)
            host->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
}

void ARP::finish()
{
}

ARP::~ARP()
{
    while (!arpCache.empty())
    {
        ARPCache::iterator i = arpCache.begin();
        delete (*i).second;
        arpCache.erase(i);
    }
    --globalArpCacheRefCnt;
    // delete my entries from the globalArpCache
    for (ARPCache::iterator it = globalArpCache.begin(); it != globalArpCache.end(); )
    {
        if (it->second->owner == this)
        {
            ARPCache::iterator cur = it++;
            delete cur->second;
            globalArpCache.erase(cur);
        }
        else
            ++it;
    }
}

void ARP::handleMessage(cMessage *msg)
{
    if (!isUp)
    {
        handleMessageWhenDown(msg);
        return;
    }

    if (msg->isSelfMessage())
    {
        requestTimedOut(msg);
    }
    else if (dynamic_cast<ARPPacket *>(msg))
    {
        ARPPacket *arp = (ARPPacket *)msg;
        processARPPacket(arp);
    }
    else // not ARP
    {
        processOutboundPacket(msg);
    }
    if (ev.isGUI())
        updateDisplayString();
}

void ARP::handleMessageWhenDown(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Model error: self msg '%s' received when protocol is down", msg->getName());
    EV << "Protocol is turned off, dropping '" << msg->getName() << "' message\n";
    delete msg;
}

bool ARP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_NETWORK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_NETWORK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }
    return true;
}

void ARP::start()
{
    ASSERT(pendingQueue.isEmpty());
    ASSERT(arpCache.empty());
    isUp = true;
}

void ARP::stop()
{
    isUp = false;
    flush();
}

void ARP::flush()
{
    while (!arpCache.empty())
    {
        ARPCache::iterator i = arpCache.begin();
        ARPCacheEntry *entry = i->second;
        if (entry->timer) {
            delete cancelEvent(entry->timer);
            entry->timer = NULL;
        }
        delete entry;
        arpCache.erase(i);
    }

    pendingQueue.clear();
}


bool ARP::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void ARP::updateDisplayString()
{
    std::stringstream os;

    os << arpCache.size() << " cache entries\nsent req:" << numRequestsSent
            << " repl:" << numRepliesSent << " fail:" << numFailedResolutions;

    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void ARP::processOutboundPacket(cMessage *msg)
{
    EV << "Packet " << msg << " arrived from IPv4\n";

    // get next hop address from control info in packet
    IPv4RoutingDecision *ctrl = check_and_cast<IPv4RoutingDecision*>(msg->removeControlInfo());
    IPv4Address nextHopAddr = ctrl->getNextHopAddr();
    InterfaceEntry *ie = ift->getInterfaceById(ctrl->getInterfaceId());
    delete ctrl;

    if (nextHopAddr.isUnspecified())
        throw cRuntimeError("Cannot start ARP, next-hop address for packet is unspecified");

    if (globalARP)
    {
        ARPCache::iterator it = globalArpCache.find(nextHopAddr);
        if (it==globalArpCache.end())
            throw cRuntimeError("Address not found in global ARP cache: %s", nextHopAddr.str().c_str());
        sendPacketToNIC(msg, ie, (*it).second->macAddress, ETHERTYPE_IPv4);
        return;
    }

    // try look up
    ARPCache::iterator it = arpCache.find(nextHopAddr);
    if (it==arpCache.end())
    {
        // no cache entry: launch ARP request
        ARPCacheEntry *entry = new ARPCacheEntry();
        entry->owner = this;
        ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(nextHopAddr, entry));
        entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        entry->ie = ie;

        EV << "Starting ARP resolution for " << nextHopAddr << "\n";
        initiateARPResolution(entry);

        // and queue up packet
        entry->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else if ((*it).second->pending)
    {
        // an ARP request is already pending for this address -- just queue up packet
        EV << "ARP resolution for " << nextHopAddr << " is pending, queueing up packet\n";
        (*it).second->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else if ((*it).second->lastUpdate+cacheTimeout<simTime())
    {
        EV << "ARP cache entry for " << nextHopAddr << " expired, starting new ARP resolution\n";

        // cache entry stale, send new ARP request
        ARPCacheEntry *entry = (*it).second;
        entry->ie = ie; // routing table may have changed
        initiateARPResolution(entry);

        // and queue up packet
        entry->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else
    {
        // valid ARP cache entry found, flag msg with MAC address and send it out
        EV << "ARP cache hit, MAC address for " << nextHopAddr << " is " << (*it).second->macAddress << ", sending packet down\n";
        sendPacketToNIC(msg, ie, (*it).second->macAddress, ETHERTYPE_IPv4);
    }
}

void ARP::initiateARPResolution(ARPCacheEntry *entry)
{
    IPv4Address nextHopAddr = entry->myIter->first;
    entry->pending = true;
    entry->numRetries = 0;
    entry->lastUpdate = 0;
    sendARPRequest(entry->ie, nextHopAddr);

    // start timer
    cMessage *msg = entry->timer = new cMessage("ARP timeout");
    msg->setContextPointer(entry);
    scheduleAt(simTime()+retryTimeout, msg);

    numResolutions++;
    emit(initiatedResolutionSignal, 1L);
}

void ARP::sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress, int etherType)
{
    // add control info with MAC address
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(macAddress);
    controlInfo->setEtherType(etherType);
    controlInfo->setInterfaceId(ie->getInterfaceId());
    msg->setControlInfo(controlInfo);

    // send out
    send(msg, netwOutGate);
}

void ARP::sendARPRequest(InterfaceEntry *ie, IPv4Address ipAddress)
{
    // find our own IPv4 address and MAC address on the given interface
    MACAddress myMACAddress = ie->getMacAddress();
    IPv4Address myIPAddress = ie->ipv4Data()->getIPAddress();

    // both must be set
    ASSERT(!myMACAddress.isUnspecified());
    ASSERT(!myIPAddress.isUnspecified());

    // fill out everything in ARP Request packet except dest MAC address
    ARPPacket *arp = new ARPPacket("arpREQ");
    arp->setByteLength(ARP_HEADER_BYTES);
    arp->setOpcode(ARP_REQUEST);
    arp->setSrcMACAddress(myMACAddress);
    arp->setSrcIPAddress(myIPAddress);
    arp->setDestIPAddress(ipAddress);

    static MACAddress broadcastAddress("ff:ff:ff:ff:ff:ff");
    sendPacketToNIC(arp, ie, broadcastAddress, ETHERTYPE_ARP);
    numRequestsSent++;
    emit(sentReqSignal, 1L);
}

void ARP::requestTimedOut(cMessage *selfmsg)
{
    ARPCacheEntry *entry = (ARPCacheEntry *)selfmsg->getContextPointer();
    entry->numRetries++;
    if (entry->numRetries < retryCount)
    {
        // retry
        IPv4Address nextHopAddr = entry->myIter->first;
        EV << "ARP request for " << nextHopAddr << " timed out, resending\n";
        sendARPRequest(entry->ie, nextHopAddr);
        scheduleAt(simTime()+retryTimeout, selfmsg);
        return;
    }

    // max retry count reached: ARP failure.
    // throw out entry from cache, delete pending messages
    MsgPtrVector& pendingPackets = entry->pendingPackets;
    EV << "ARP timeout, max retry count " << retryCount << " for "
       << entry->myIter->first << " reached. Dropping " << pendingPackets.size()
       << " waiting packets from the queue\n";
    while (!pendingPackets.empty())
    {
        MsgPtrVector::iterator i = pendingPackets.begin();
        cMessage *msg = (*i);
        pendingPackets.erase(i);
        pendingQueue.remove(msg);
        delete msg;
    }
    delete selfmsg;
    arpCache.erase(entry->myIter);
    delete entry;
    numFailedResolutions++;
    emit(failedResolutionSignal, 1L);
}


bool ARP::addressRecognized(IPv4Address destAddr, InterfaceEntry *ie)
{
    if (rt->isLocalAddress(destAddr)) {
        return true;
    }
    else if (respondToProxyARP) {
        // respond to Proxy ARP request: if we can route this packet (and the
        // output port is different from this one), say yes
        InterfaceEntry *rtie = rt->getInterfaceForDestAddr(destAddr);
        return rtie!=NULL && rtie!=ie;
    }
    else {
        return false;
    }
}

void ARP::dumpARPPacket(ARPPacket *arp)
{
    EV << (arp->getOpcode()==ARP_REQUEST ? "ARP_REQ" : arp->getOpcode()==ARP_REPLY ? "ARP_REPLY" : "unknown type")
       << "  src=" << arp->getSrcIPAddress() << " / " << arp->getSrcMACAddress()
       << "  dest=" << arp->getDestIPAddress() << " / " << arp->getDestMACAddress() << "\n";
}


void ARP::processARPPacket(ARPPacket *arp)
{
    EV << "ARP packet " << arp << " arrived:\n";
    dumpARPPacket(arp);

    // extract input port
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(arp->removeControlInfo());
    InterfaceEntry *ie = ift->getInterfaceById(ctrl->getInterfaceId());
    delete ctrl;

    //
    // Recipe a'la RFC 826:
    //
    // ?Do I have the hardware type in ar$hrd?
    // Yes: (almost definitely)
    //   [optionally check the hardware length ar$hln]
    //   ?Do I speak the protocol in ar$pro?
    //   Yes:
    //     [optionally check the protocol length ar$pln]
    //     Merge_flag := false
    //     If the pair <protocol type, sender protocol address> is
    //         already in my translation table, update the sender
    //         hardware address field of the entry with the new
    //         information in the packet and set Merge_flag to true.
    //     ?Am I the target protocol address?
    //     Yes:
    //       If Merge_flag is false, add the triplet <protocol type,
    //           sender protocol address, sender hardware address> to
    //           the translation table.
    //       ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
    //       Yes:
    //         Swap hardware and protocol fields, putting the local
    //             hardware and protocol addresses in the sender fields.
    //         Set the ar$op field to ares_op$REPLY
    //         Send the packet to the (new) target hardware address on
    //             the same hardware on which the request was received.
    //

    MACAddress srcMACAddress = arp->getSrcMACAddress();
    IPv4Address srcIPAddress = arp->getSrcIPAddress();

    if (srcMACAddress.isUnspecified())
        error("wrong ARP packet: source MAC address is empty");
    if (srcIPAddress.isUnspecified())
        error("wrong ARP packet: source IPv4 address is empty");

    bool mergeFlag = false;
    // "If ... sender protocol address is already in my translation table"
    ARPCache::iterator it = arpCache.find(srcIPAddress);
    if (it!=arpCache.end())
    {
        // "update the sender hardware address field"
        ARPCacheEntry *entry = (*it).second;
        updateARPCache(entry, srcMACAddress);
        mergeFlag = true;
    }

    // "?Am I the target protocol address?"
    // if Proxy ARP is enabled, we also have to reply if we're a router to the dest IPv4 address
    if (addressRecognized(arp->getDestIPAddress(), ie))
    {
        // "If Merge_flag is false, add the triplet protocol type, sender
        // protocol address, sender hardware address to the translation table"
        if (!mergeFlag)
        {
            ARPCacheEntry *entry;
            if (it!=arpCache.end())
            {
                entry = (*it).second;
            }
            else
            {
                entry = new ARPCacheEntry();
                ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(srcIPAddress, entry));
                entry->myIter = where;
                entry->ie = ie;

                entry->pending = false;
                entry->timer = NULL;
                entry->numRetries = 0;
            }
            updateARPCache(entry, srcMACAddress);
        }

        // "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
        switch (arp->getOpcode())
        {
            case ARP_REQUEST:
            {
                EV << "Packet was ARP REQUEST, sending REPLY\n";

                // find our own IPv4 address and MAC address on the given interface
                MACAddress myMACAddress = ie->getMacAddress();
                IPv4Address myIPAddress = ie->ipv4Data()->getIPAddress();

                // "Swap hardware and protocol fields", etc.
                arp->setName("arpREPLY");
                IPv4Address origDestAddress = arp->getDestIPAddress();
                arp->setDestIPAddress(srcIPAddress);
                arp->setDestMACAddress(srcMACAddress);
                arp->setSrcIPAddress(origDestAddress);
                arp->setSrcMACAddress(myMACAddress);
                arp->setOpcode(ARP_REPLY);
                delete arp->removeControlInfo();
                sendPacketToNIC(arp, ie, srcMACAddress, ETHERTYPE_ARP);
                numRepliesSent++;
                emit(sentReplySignal, 1L);
                break;
            }
            case ARP_REPLY:
            {
                EV << "Discarding packet\n";
                delete arp;
                break;
            }
            case ARP_RARP_REQUEST: throw cRuntimeError("RARP request received: RARP is not supported");
            case ARP_RARP_REPLY: throw cRuntimeError("RARP reply received: RARP is not supported");
            default: throw cRuntimeError("Unsupported opcode %d in received ARP packet", arp->getOpcode());
        }
    }
    else
    {
        // address not recognized
        EV << "IPv4 address " << arp->getDestIPAddress() << " not recognized, dropping ARP packet\n";
        delete arp;
    }
}

void ARP::updateARPCache(ARPCacheEntry *entry, const MACAddress& macAddress)
{
    EV << "Updating ARP cache entry: " << entry->myIter->first << " <--> " << macAddress << "\n";

    // update entry
    if (entry->pending)
    {
        entry->pending = false;
        delete cancelEvent(entry->timer);
        entry->timer = NULL;
        entry->numRetries = 0;
    }
    entry->macAddress = macAddress;
    entry->lastUpdate = simTime();

    // process queued packets
    MsgPtrVector& pendingPackets = entry->pendingPackets;
    while (!pendingPackets.empty())
    {
        MsgPtrVector::iterator i = pendingPackets.begin();
        cMessage *msg = (*i);
        pendingPackets.erase(i);
        pendingQueue.remove(msg);
        EV << "Sending out queued packet " << msg << "\n";
        sendPacketToNIC(msg, entry->ie, macAddress, ETHERTYPE_IPv4);
    }
}

MACAddress ARP::getDirectAddressResolution(const IPv4Address & add) const
{
    ARPCache::const_iterator it;
    if (globalARP)
    {
        it = globalArpCache.find(add);
        if (it!=globalArpCache.end())
            return (*it).second->macAddress;
    }
    else
    {
        it = arpCache.find(add);
        if (it!=arpCache.end())
        {
            if ((*it).second->pending)
            {}  // has pending packets
            else if ((*it).second->lastUpdate + cacheTimeout < simTime())
            {}  // expired
            else
                return (*it).second->macAddress;
        }
    }
    return MACAddress::UNSPECIFIED_ADDRESS;
}

IPv4Address ARP::getInverseAddressResolution(const MACAddress &add) const
{
    if (add.isUnspecified())
        return IPv4Address::UNSPECIFIED_ADDRESS;

    ARPCache::const_iterator it;
    if (globalARP)
    {
        for (it = globalArpCache.begin(); it!=globalArpCache.end(); it++)
            if ((*it).second->macAddress==add)
                return (*it).first;
    }
    else
    {
        simtime_t now = simTime();
        for (it = arpCache.begin(); it!=arpCache.end(); it++)
            if ((*it).second->macAddress==add)
            {
                if ((*it).second->lastUpdate + cacheTimeout >= now)
                    return (*it).first;
            }
    }
    return IPv4Address::UNSPECIFIED_ADDRESS;
}

void ARP::setChangeAddress(const IPv4Address &oldAddress)
{
    Enter_Method_Silent();
    ARPCache::iterator it;

    if (oldAddress.isUnspecified())
        return;
    if (globalARP)
    {
        it = globalArpCache.find(oldAddress);
        if (it!=globalArpCache.end())
        {
            ARPCacheEntry *entry = (*it).second;
            globalArpCache.erase(it);
            entry->pending = false;
            entry->timer = NULL;
            entry->numRetries = 0;
            IPv4Address nextHopAddr = entry->ie->ipv4Data()->getIPAddress();
            ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(nextHopAddr, entry));
            entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        }
    }
}


void ARP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    // host associated. Link is up. Change the state to init.
    if (signalID == NF_INTERFACE_IPv4CONFIG_CHANGED)
    {
        const InterfaceEntryChangeDetails *iecd = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        InterfaceEntry *ie = iecd->getInterfaceEntry();
        // rebuild the arp cache
        if (ie->isLoopback())
            return;
        ARPCache::iterator it;
        for (it=globalArpCache.begin(); it!=globalArpCache.end(); ++it)
        {
            if (it->second->ie == ie)
                break;
        }
        ARPCacheEntry *entry = NULL;
        if (it == globalArpCache.end())
        {
            if (!ie->ipv4Data() || ie->ipv4Data()->getIPAddress().isUnspecified())
                return; // if the address is not defined it isn't included in the global cache
            entry = new ARPCacheEntry();
            entry->owner = this;
            entry->ie = ie;
        }
        else
        {
            // actualize
            entry = it->second;
            ASSERT(entry->owner == this);
            globalArpCache.erase(it);
            if (!ie->ipv4Data() || ie->ipv4Data()->getIPAddress().isUnspecified())
            {
                delete entry;
                return; // if the address is not defined it isn't included in the global cache
            }
        }
        entry->pending = false;
        entry->timer = NULL;
        entry->numRetries = 0;
        entry->macAddress = ie->getMacAddress();
        IPv4Address ipAddr = ie->ipv4Data()->getIPAddress();
        ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(),std::make_pair(ipAddr, entry));
        ASSERT(where->second == entry);
        entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
    }
}
