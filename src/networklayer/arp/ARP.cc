/*
 * Copyright (C) 2004 Andras Varga
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
#include "IPv4InterfaceData.h"
#include "Ieee802Ctrl_m.h"


static std::ostream& operator<< (std::ostream& out, cMessage *msg)
{
    out << "(" << msg->getClassName() << ")" << msg->getFullName();
    return out;
}

static std::ostream& operator<< (std::ostream& out, const ARP::ARPCacheEntry& e)
{
    if (e.pending)
        out << "pending (" << e.numRetries << " retries)";
    else
        out << "MAC:" << e.macAddress << "  age:" << floor(simTime()-e.lastUpdate) << "s";
    return out;
}


Define_Module (ARP);

void ARP::initialize()
{
    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    nicOutBaseGateId = gateSize("nicOut")==0 ? -1 : gate("nicOut",0)->getId();

    retryTimeout = par("retryTimeout");
    retryCount = par("retryCount");
    cacheTimeout = par("cacheTimeout");
    doProxyARP = par("proxyARP");

    pendingQueue.setName("pendingQueue");

    // init statistics
    numRequestsSent = numRepliesSent = 0;
    numResolutions = numFailedResolutions = 0;
    WATCH(numRequestsSent);
    WATCH(numRepliesSent);
    WATCH(numResolutions);
    WATCH(numFailedResolutions);

    WATCH_PTRMAP(arpCache);
}

void ARP::finish()
{
    recordScalar("ARP requests sent", numRequestsSent);
    recordScalar("ARP replies sent", numRepliesSent);
    recordScalar("ARP resolutions", numResolutions);
    recordScalar("failed ARP resolutions", numFailedResolutions);
}

ARP::~ARP()
{
    while (!arpCache.empty())
    {
        ARPCache::iterator i = arpCache.begin();
        delete (*i).second;
        arpCache.erase(i);
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
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision*>(msg->removeControlInfo());
    IPAddress nextHopAddr = controlInfo->getNextHopAddr();
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
        IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);
        nextHopAddr = datagram->getDestAddress();
        EV << "no next-hop address, using destination address " << nextHopAddr << " (proxy ARP)\n";
    }

    //
    // Handle multicast IP addresses. RFC 1112, section 6.4 says:
    // "An IP host group address is mapped to an Ethernet multicast address
    // by placing the low-order 23 bits of the IP address into the low-order
    // 23 bits of the Ethernet multicast address 01-00-5E-00-00-00 (hex).
    // Because there are 28 significant bits in an IP host group address,
    // more than one host group address may map to the same Ethernet multicast
    // address."
    //
    if (nextHopAddr.isMulticast())
    {
        // FIXME: we do a simpler solution right now: send to the Broadcast MAC address
        EV << "destination address is multicast, sending packet to broadcast MAC address\n";
        static MACAddress broadcastAddr("FF:FF:FF:FF:FF:FF");
        sendPacketToNIC(msg, ie, broadcastAddr);
        return;
#if 0
        // experimental RFC 1112 code
        // TBD needs counterpart to be implemented in EtherMAC processReceivedDataFrame().
        unsigned char macBytes[6];
        macBytes[0] = 0x01;
        macBytes[1] = 0x00;
        macBytes[2] = 0x5e;
        macBytes[3] = nextHopAddr.getDByte(1) & 0x7f;
        macBytes[4] = nextHopAddr.getDByte(2);
        macBytes[5] = nextHopAddr.getDByte(3);
        MACAddress multicastMacAddr;
        multicastMacAddr.setAddressBytes(bytes);
        sendPacketToNIC(msg, ie, multicastMacAddr);
        return;
#endif
    }

    // try look up
    ARPCache::iterator it = arpCache.find(nextHopAddr);
    //ASSERT(it==arpCache.end() || ie==(*it).second->ie); // verify: if arpCache gets keyed on InterfaceEntry* too, this becomes unnecessary
    if (it==arpCache.end())
    {
        // no cache entry: launch ARP request
        ARPCacheEntry *entry = new ARPCacheEntry();
        ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(nextHopAddr,entry));
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
        sendPacketToNIC(msg, ie, (*it).second->macAddress);
    }
}

void ARP::initiateARPResolution(ARPCacheEntry *entry)
{
    IPAddress nextHopAddr = entry->myIter->first;
    entry->pending = true;
    entry->numRetries = 0;
    entry->lastUpdate = 0;
    sendARPRequest(entry->ie, nextHopAddr);

    // start timer
    cMessage *msg = entry->timer = new cMessage("ARP timeout");
    msg->setContextPointer(entry);
    scheduleAt(simTime()+retryTimeout, msg);

    numResolutions++;
}

void ARP::sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress)
{
    // add control info with MAC address
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(macAddress);
    msg->setControlInfo(controlInfo);

    // send out
    send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
}

void ARP::sendARPRequest(InterfaceEntry *ie, IPAddress ipAddress)
{
    // find our own IP address and MAC address on the given interface
    MACAddress myMACAddress = ie->getMacAddress();
    IPAddress myIPAddress = ie->ipv4Data()->getIPAddress();

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
    sendPacketToNIC(arp, ie, broadcastAddress);
    numRequestsSent++;
}

void ARP::requestTimedOut(cMessage *selfmsg)
{
    ARPCacheEntry *entry = (ARPCacheEntry *)selfmsg->getContextPointer();
    entry->numRetries++;
    if (entry->numRetries < retryCount)
    {
        // retry
        IPAddress nextHopAddr = entry->myIter->first;
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
}


bool ARP::addressRecognized(IPAddress destAddr, InterfaceEntry *ie)
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
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision*>(arp->removeControlInfo());
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
    IPAddress srcIPAddress = arp->getSrcIPAddress();

    if (srcMACAddress.isUnspecified())
        error("wrong ARP packet: source MAC address is empty");
    if (srcIPAddress.isUnspecified())
        error("wrong ARP packet: source IP address is empty");

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
    // if Proxy ARP is enabled, we also have to reply if we're a router to the dest IP address
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
                ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(srcIPAddress,entry));
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

                // find our own IP address and MAC address on the given interface
                MACAddress myMACAddress = ie->getMacAddress();
                IPAddress myIPAddress = ie->ipv4Data()->getIPAddress();

                // "Swap hardware and protocol fields", etc.
                arp->setName("arpREPLY");
                IPAddress origDestAddress = arp->getDestIPAddress();
                arp->setDestIPAddress(srcIPAddress);
                arp->setDestMACAddress(srcMACAddress);
                arp->setSrcIPAddress(origDestAddress);
                arp->setSrcMACAddress(myMACAddress);
                arp->setOpcode(ARP_REPLY);
                delete arp->removeControlInfo();
                sendPacketToNIC(arp, ie, srcMACAddress);
                numRepliesSent++;
                break;
            }
            case ARP_REPLY:
            {
                EV << "Discarding packet\n";
                delete arp;
                break;
            }
            case ARP_RARP_REQUEST: error("RARP request received: RARP is not supported");
            case ARP_RARP_REPLY: error("RARP reply received: RARP is not supported");
            default: error("Unsupported opcode %d in received ARP packet",arp->getOpcode());
        }
    }
    else
    {
        // address not recognized
        EV << "IP address " << arp->getDestIPAddress() << " not recognized, dropping ARP packet\n";
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
        sendPacketToNIC(msg, entry->ie, macAddress);
    }
}


