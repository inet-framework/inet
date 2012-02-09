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
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#include "ARPPacket_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"


simsignal_t ARP::sentReqSignal = SIMSIGNAL_NULL;
simsignal_t ARP::sentReplySignal = SIMSIGNAL_NULL;
simsignal_t ARP::failedResolutionSignal = SIMSIGNAL_NULL;
simsignal_t ARP::initiatedResolutionSignal = SIMSIGNAL_NULL;

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
    if (stage==0)
    {
        sentReqSignal = registerSignal("sentReq");
        sentReplySignal = registerSignal("sentReply");
        initiatedResolutionSignal = registerSignal("initiatedResolution");
        failedResolutionSignal = registerSignal("failedResolution");
    }

    if (stage==4)
    {
        ift = InterfaceTableAccess().get();
        rt = RoutingTableAccess().get();

        nicOutBaseGateId = gateSize("nicOut")==0 ? -1 : gate("nicOut", 0)->getId();

        retryTimeout = par("retryTimeout");
        retryCount = par("retryCount");
        cacheTimeout = par("cacheTimeout");
        doProxyARP = par("proxyARP");
        globalARP = par("globalARP");

        pendingQueue.setName("pendingQueue");

        // init statistics
        numRequestsSent = numRepliesSent = 0;
        numResolutions = numFailedResolutions = 0;
        WATCH(numRequestsSent);
        WATCH(numRepliesSent);
        WATCH(numResolutions);
        WATCH(numFailedResolutions);

        WATCH_PTRMAP(arpCache);
        WATCH_PTRMAP(globalArpCache);

        // initialize global cache
        for (int i=0; i<ift->getNumInterfaces(); i++)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isLoopback())
                continue;
            ARPCacheEntry *entry = new ARPCacheEntry();
            entry->ie = ie;
            entry->pending = false;
            entry->timer = NULL;
            entry->numRetries = 0;
            entry->macAddress = ie->getMacAddress();
            IPv4Address nextHopAddr = ie->ipv4Data()->getIPAddress();
            ARPCache::iterator where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(nextHopAddr, entry));
            entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        }
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

    if (--globalArpCacheRefCnt != 0)
        return;

    while (!globalArpCache.empty())
    {
        ARPCache::iterator i = globalArpCache.begin();
        delete (*i).second;
        globalArpCache.erase(i);
    }
}

void ARP::handleMessage(cMessage *msg)
{
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

void ARP::updateDisplayString()
{
    std::stringstream os;

    os << arpCache.size() << " cache entries\nsent req:" << numRequestsSent
            << " repl:" << numRepliesSent << " fail:" << numFailedResolutions;

    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void ARP::processOutboundPacket(cMessage *msg)
{
    EV << "Packet " << msg << " arrived from higher layer, ";

    // get next hop address from control info in packet
    IPv4RoutingDecision *controlInfo = check_and_cast<IPv4RoutingDecision*>(msg->removeControlInfo());
    IPv4Address nextHopAddr = controlInfo->getNextHopAddr();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    delete controlInfo;

    // if output interface is not broadcast, don't bother with ARP
    if (!ie->isBroadcast())
    {
        EV << "output interface " << ie->getName() << " is not broadcast, skipping ARP\n";
        send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
        return;
    }

    // determine what address to look up in ARP cache
    if (!nextHopAddr.isUnspecified())
    {
        EV << "using next-hop address " << nextHopAddr << "\n";
    }
    else
    {
        // try proxy ARP
        IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(msg);
        nextHopAddr = datagram->getDestAddress();
        EV << "no next-hop address, using destination address " << nextHopAddr << " (proxy ARP)\n";
    }

    if (nextHopAddr.isLimitedBroadcastAddress() ||
            nextHopAddr == ie->ipv4Data()->getIPAddress().getBroadcastAddress(ie->ipv4Data()->getNetmask())) // also include the network broadcast
    {
        EV << "destination address is broadcast, sending packet to broadcast MAC address\n";
        sendPacketToNIC(msg, ie, MACAddress::BROADCAST_ADDRESS, ETHERTYPE_IPv4);
        return;
    }

    if (nextHopAddr.isMulticast())
    {
        MACAddress macAddr = mapMulticastAddress(nextHopAddr);
        EV << "destination address is multicast, sending packet to MAC address " << macAddr << "\n";
        sendPacketToNIC(msg, ie, macAddr, ETHERTYPE_IPv4);
        return;
    }

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
    //ASSERT(it==arpCache.end() || ie==(*it).second->ie); // verify: if arpCache gets keyed on InterfaceEntry* too, this becomes unnecessary
    if (it==arpCache.end())
    {
        // no cache entry: launch ARP request
        ARPCacheEntry *entry = new ARPCacheEntry();
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

// see  RFC 1112, section 6.4
MACAddress ARP::mapMulticastAddress(IPv4Address addr)
{
    ASSERT(addr.isMulticast());

    MACAddress macAddr;
    macAddr.setAddressByte(0, 0x01);
    macAddr.setAddressByte(1, 0x00);
    macAddr.setAddressByte(2, 0x5e);
    macAddr.setAddressByte(3, addr.getDByte(1) & 0x7f);
    macAddr.setAddressByte(4, addr.getDByte(2));
    macAddr.setAddressByte(5, addr.getDByte(3));
    return macAddr;
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
    msg->setControlInfo(controlInfo);

    // send out
    send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
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
    if (rt->isLocalAddress(destAddr))
        return true;

    // respond to Proxy ARP request: if we can route this packet (and the
    // output port is different from this one), say yes
    if (!doProxyARP)
        return false;
    InterfaceEntry *rtie = rt->getInterfaceForDestAddr(destAddr);
    return rtie!=NULL && rtie!=ie;
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
    IPv4RoutingDecision *controlInfo = check_and_cast<IPv4RoutingDecision*>(arp->removeControlInfo());
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    delete controlInfo;

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

const MACAddress ARP::getDirectAddressResolution(const IPv4Address & add) const
{
    ARPCache::const_iterator it;
    MACAddress address = MACAddress::UNSPECIFIED_ADDRESS;
    if (globalARP)
    {
        it = globalArpCache.find(add);
        if (it!=globalArpCache.end())
            address = (*it).second->macAddress;
    }
    else
    {
        it = arpCache.find(add);
        if (it!=arpCache.end())
            address = (*it).second->macAddress;
    }
    return address;
}

const IPv4Address ARP::getInverseAddressResolution(const MACAddress &add) const
{
    IPv4Address address;
    ARPCache::const_iterator it;
    if (globalARP)
    {
        for (it = globalArpCache.begin(); it!=globalArpCache.end(); it++)
            if ((*it).second->macAddress==add)
            {
                address = (*it).first;
                return address;
            }
    }
    else
    {
        for (it = arpCache.begin(); it!=arpCache.end(); it++)
            if ((*it).second->macAddress==add)
            {
                address = (*it).first;
                return address;
            }
    }
    return address;
}

void ARP::setChangeAddress(const IPv4Address &oldAddress)
{
    Enter_Method_Silent();
    ARPCache::iterator it;
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
