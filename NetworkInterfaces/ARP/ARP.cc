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

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <omnetpp.h>
#include "ipsuite_defs.h"
#include "IPAddress.h"
#include "ARPPacket_m.h"
#include "EtherCtrl_m.h"
#include "IPControlInfo_m.h"
#include "IPDatagram.h"
#include "RoutingTableAccess.h"
#include "EtherMAC.h"


static std::ostream& operator<< (std::ostream& ev, cMessage *msg)
{
    ev << "(" << msg->className() << ")" << msg->fullName();
    return ev;
}


/**
 * ARP implementation.
 */
class ARP : public cSimpleModule
{
  protected:
    simtime_t retryTimeout;
    int retryCount;
    simtime_t cacheTimeout;

    IPAddress myIPAddress;
    MACAddress myMACAddress;

    long numResolutions;
    long numFailedResolutions;
    long numRequestsSent;
    long numRepliesSent;

    struct ARPCacheEntry;
    typedef std::map<IPAddress, ARPCacheEntry*> ARPCache;
    typedef std::vector<cMessage*> MsgPtrVector;

    // IPAddress -> MACAddress table
    struct ARPCacheEntry
    {
        bool pending; // true if resolution is pending
        MACAddress macAddress;  // MAC address
        simtime_t lastUpdate;  // entries should time out after cacheTimeout
        int numRetries; // if pending==true: 0 after first ARP request, 1 after second, etc.
        cMessage *timer;  // if pending==true: request timeout msg
        MsgPtrVector pendingPackets;  // if pending==true: ptrs to packets waiting for resolution
                                      // (packets are owned by pendingQueue)
        ARPCache::iterator myIter;  // iterator pointing to this entry
    };

    ARPCache arpCache;

    cQueue pendingQueue; // outbound packets waiting for ARP resolution

    RoutingTableAccess routingTableAccess; // for Proxy ARP

  public:
    Module_Class_Members(ARP,cSimpleModule,0);
    ~ARP();

    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    void processInboundPacket(cMessage *msg);
    void processOutboundPacket(cMessage *msg);
    void sendPacketToMAC(cMessage *msg, const MACAddress& macAddress);

    void initiateARPResolution(IPAddress nextHopAddr, ARPCacheEntry *entry);
    void sendARPRequest(IPAddress ipAddress);
    void requestTimedOut(cMessage *selfmsg);
    bool addressRecognized(IPAddress destAddr);
    void processARPPacket(ARPPacket *arp);
    void updateARPCache(ARPCacheEntry *entry, const MACAddress& macAddress);
};

Define_Module (ARP);

void ARP::initialize(int stage)
{
    // we have to wait until interfaces are registered and address auto-assignment takes place
    if (stage!=3)
        return;

    retryTimeout = par("retryTimeout");
    retryCount = par("retryCount");
    cacheTimeout = par("cacheTimeout");

    pendingQueue.setName("pendingQueue");

    // fill in myIPAddress
    // TBD find out something more elegant
    char *interfaceName = new char[strlen(parentModule()->fullName())+1];
    char *d=interfaceName;
    for (const char *s=owner()->fullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';
    myIPAddress = routingTableAccess.get()->interfaceByName(interfaceName)->inetAddr;
    delete [] interfaceName;

    // fill in myMACAddress
    myMACAddress = ((EtherMAC *)parentModule()->submodule("mac"))->getMACAddress();

    // init statistics
    numRequestsSent = numRepliesSent = 0;
    numResolutions = numFailedResolutions = 0;
    WATCH(numRequestsSent);
    WATCH(numRepliesSent);
    WATCH(numResolutions);
    WATCH(numFailedResolutions);
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
    EV << "Packet " << msg << " arrived from higher layer\n";

    // get control info from packet
    IPRoutingDecision *controlInfo = check_and_cast<IPRoutingDecision*>(msg->removeControlInfo());
    IPAddress nextHopAddr = controlInfo->nextHopAddr();
    delete controlInfo;

    if (nextHopAddr.isNull())
    {
        // try proxy ARP
        EV << "No next hop address, trying Proxy ARP\n";
        IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);
        nextHopAddr = datagram->destAddress();
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
        EV << "MAC address for " << nextHopAddr << " is " << (*it).second->macAddress << ", sending packet down\n";
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
    ARPPacket *arp = new ARPPacket();
    arp->setOpcode(ARP_REQUEST);
    // arp->setHardwareType();
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

    // respond to Proxy ARP request: if we can route this packet, say yes
    RoutingTable *rt = routingTableAccess.get();
    bool routable = (rt->outputPortNo(destAddr)!=-1);
    return routable;
}

void ARP::processARPPacket(ARPPacket *arp)
{
    EV << "ARP packet " << arp << " arrived\n";

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
                // "Swap hardware and protocol fields", etc.
                arp->setDestIPAddress(srcIPAddress);
                arp->setDestMACAddress(srcMACAddress);
                arp->setSrcIPAddress(myIPAddress);
                arp->setSrcMACAddress(myMACAddress);
                arp->setOpcode(ARP_REPLY);
                delete arp->removeControlInfo();
                sendPacketToMAC(arp, srcMACAddress);
                numRepliesSent++;
            }
            case ARP_REPLY: delete arp; break;
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
    EV << "Updating ARP cache entry: " << entry->myIter->first << " --> " << macAddress << "\n";

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
        sendPacketToMAC(msg, macAddress);
    }
}


