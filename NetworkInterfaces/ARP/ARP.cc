/*
 * Copyright (C) 2004 Andras Varga
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "ARP.h"
#include "stlwatch.h"


static std::ostream& operator<< (std::ostream& ev, cMessage *msg)
{
    ev << "(" << msg->className() << ")" << msg->fullName();
    return ev;
}

static std::ostream& operator<< (std::ostream& ev, const ARP::ARPCacheEntry& e)
{
    if (e.pending)
        ev << "pending (" << e.numRetries << " retries)";
    else
        ev << "MAC:" << e.macAddress << "  age:" << floor(simulation.simTime()-e.lastUpdate) << "s";
    return ev;
}


Define_Module (ARP);

void ARP::initialize(int stage)
{
    if (stage==0)
    {
        // register interface in 1st stage
        interfaceEntry = registerInterface(100000); // FIXME hardcoded 100 Mbps
        return;
    }

    // with the rest we have to wait until address auto-assignment takes place
    if (stage!=3)
        return;

    retryTimeout = par("retryTimeout");
    retryCount = par("retryCount");
    cacheTimeout = par("cacheTimeout");
    doProxyARP = par("proxyARP");

    pendingQueue.setName("pendingQueue");

    // fill in myIPAddress and myMACAddress
    myIPAddress = interfaceEntry->inetAddr;
    myMACAddress = ((EtherMAC *)parentModule()->submodule("mac"))->getMACAddress();

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
    else if (msg->arrivedOn("hwIn"))
    {
        if (dynamic_cast<ARPPacket *>(msg))
        {
            ARPPacket *arp = (ARPPacket *)msg;
            processARPPacket(arp);
        }
        else // not ARP
        {
            processInboundPacket(msg);
        }
    }
    else
    {
        processOutboundPacket(msg);
    }

    if (ev.isGUI())
        updateDisplayString();
}

void ARP::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "%d cache entries\nsent req:%d repl:%d fail:%d",
                 arpCache.size(), numRequestsSent, numRepliesSent, numFailedResolutions);
    displayString().setTagArg("t",0,buf);
}

void ARP::processInboundPacket(cMessage *msg)
{
    // remove control info from packet, and send it up
    EV << "Packet " << msg << " arrived from network, sending up\n";
    delete msg->removeControlInfo();
    send(msg,"hlOut");
}

void ARP::processOutboundPacket(cMessage *msg)
{
    EV << "Packet " << msg << " arrived from higher layer, ";

    // get next hop address from optional control info in packet
    IPAddress nextHopAddr;
    if (msg->controlInfo())
    {
        IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision*>(msg->removeControlInfo());
        nextHopAddr = controlInfo->nextHopAddr();
        delete controlInfo;
    }

    if (nextHopAddr.isNull())
    {
        // try proxy ARP
        IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);
        nextHopAddr = datagram->destAddress();
        EV << "destination address " << nextHopAddr << " (no next-hop address)\n";
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
        sendPacketToMAC(msg, broadcastAddr);
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
        sendPacketToMAC(msg, multicastMacAddr);
        return;
#endif
    }

    // try look up
    ARPCache::iterator it = arpCache.find(nextHopAddr);
    if (it==arpCache.end())
    {
        // no cache entry: launch ARP request
        ARPCacheEntry *entry = new ARPCacheEntry();
        ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(nextHopAddr,entry));
        entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"

        EV << "Starting ARP resolution for " << nextHopAddr << "\n";
        initiateARPResolution(nextHopAddr,entry);

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
        initiateARPResolution(nextHopAddr,entry);

        // and queue up packet
        entry->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else
    {
        // valid ARP cache entry found, flag msg with MAC address and send it out
        EV << "ARP cache hit, MAC address for " << nextHopAddr << " is " << (*it).second->macAddress << ", sending packet down\n";
        sendPacketToMAC(msg, (*it).second->macAddress);
    }
}

void ARP::initiateARPResolution(IPAddress nextHopAddr, ARPCacheEntry *entry)
{
    entry->pending = true;
    entry->numRetries = 0;
    entry->lastUpdate = 0;
    sendARPRequest(nextHopAddr);

    // start timer
    cMessage *msg = entry->timer = new cMessage("ARP timeout");
    msg->setContextPointer(entry);
    scheduleAt(simTime()+retryTimeout, msg);

    numResolutions++;
}

void ARP::sendPacketToMAC(cMessage *msg, const MACAddress& macAddress)
{
    // add control info with MAC address
    EtherCtrl *controlInfo = new EtherCtrl();
    controlInfo->setDest(macAddress);
    msg->setControlInfo(controlInfo);

    // send out
    send(msg,"hwOut");
}

void ARP::sendARPRequest(IPAddress ipAddress)
{
    // fill out everything except dest MAC address
    ARPPacket *arp = new ARPPacket("arpREQ");
    arp->setOpcode(ARP_REQUEST);
    arp->setSrcMACAddress(myMACAddress);
    arp->setSrcIPAddress(myIPAddress);
    arp->setDestIPAddress(ipAddress);

    static MACAddress broadcastAddress("ff:ff:ff:ff:ff:ff:ff:ff");
    sendPacketToMAC(arp, broadcastAddress);
    numRequestsSent++;
}

void ARP::requestTimedOut(cMessage *selfmsg)
{
    ARPCacheEntry *entry = (ARPCacheEntry *)selfmsg->contextPointer();
    entry->numRetries++;
    if (entry->numRetries < retryCount)
    {
        // retry
        IPAddress nextHopAddr = entry->myIter->first;
        EV << "ARP request for " << nextHopAddr << " timed out, resending\n";
        sendARPRequest(nextHopAddr);
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


bool ARP::addressRecognized(IPAddress destAddr)
{
    if (destAddr==myIPAddress)
        return true;

    // respond to Proxy ARP request: if we can route this packet (and the
    // output port is different from this one), say yes
    if (!doProxyARP)
        return false;
    RoutingTable *rt = routingTableAccess.get();
    int outputPort = rt->outputPortNo(destAddr);
    return outputPort!=-1 && outputPort!=interfaceEntry->outputPort;
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
    if (addressRecognized(arp->getDestIPAddress()))
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

                // "Swap hardware and protocol fields", etc.
                arp->setName("arpREPLY");
                IPAddress origDestAddress = arp->getDestIPAddress();
                arp->setDestIPAddress(srcIPAddress);
                arp->setDestMACAddress(srcMACAddress);
                arp->setSrcIPAddress(origDestAddress);
                arp->setSrcMACAddress(myMACAddress);
                arp->setOpcode(ARP_REPLY);
                delete arp->removeControlInfo();
                sendPacketToMAC(arp, srcMACAddress);
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
        sendPacketToMAC(msg, macAddress);
    }
}

InterfaceEntry *ARP::registerInterface(double datarate)
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: NetworkInterface module's name without special characters ([])
    // --> Emin : Parent module name is used since EtherMAC belongs to EthernetInterface.
    char *interfaceName = new char[strlen(parentModule()->fullName())+1];
    char *d=interfaceName;
    for (const char *s=parentModule()->fullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->name = interfaceName;
    delete [] interfaceName;

    // port: index of gate where parent module's "netwIn" is connected (in IP)
    int outputPort = parentModule()->gate("netwIn")->sourceGate()->index();
    e->outputPort = outputPort;

    // we don't know IP address and netmask, it'll probably come from routing table file

    // MTU is 1500 on Ethernet
    e->mtu = 1500;

    // metric: some hints: OSPF cost (2e9/bps value), MS KB article Q299540, ...
    e->metric = (int)ceil(2e9/datarate); // use OSPF cost as default

    // capabilities
    e->multicast = true;
    e->pointToPoint = false;

    // multicast groups
    // TBD

    // add
    RoutingTableAccess routingTableAccess;
    routingTableAccess.get()->addInterface(e);

    return e;
}
