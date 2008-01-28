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

#ifndef __ARP_H__
#define __ARP_H__

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <omnetpp.h>
#include "IPAddress.h"
#include "ARPPacket_m.h"
#include "IPControlInfo.h"
#include "IPDatagram.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"



/**
 * ARP implementation.
 */
class INET_API ARP : public cSimpleModule
{
  public:
    struct ARPCacheEntry;
    typedef std::map<IPAddress, ARPCacheEntry*> ARPCache;
    typedef std::vector<cMessage*> MsgPtrVector;

    // IPAddress -> MACAddress table
    // TBD should we key it on (IPAddress, InterfaceEntry*)?
    struct ARPCacheEntry
    {
        InterfaceEntry *ie; // NIC to send the packet to
        bool pending; // true if resolution is pending
        MACAddress macAddress;  // MAC address
        simtime_t lastUpdate;  // entries should time out after cacheTimeout
        int numRetries; // if pending==true: 0 after first ARP request, 1 after second, etc.
        cMessage *timer;  // if pending==true: request timeout msg
        MsgPtrVector pendingPackets;  // if pending==true: ptrs to packets waiting for resolution
                                      // (packets are owned by pendingQueue)
        ARPCache::iterator myIter;  // iterator pointing to this entry
    };

  protected:
    simtime_t retryTimeout;
    int retryCount;
    simtime_t cacheTimeout;
    bool doProxyARP;

    long numResolutions;
    long numFailedResolutions;
    long numRequestsSent;
    long numRepliesSent;

    ARPCache arpCache;

    cQueue pendingQueue; // outbound packets waiting for ARP resolution

    InterfaceTable *ift;
    RoutingTable *rt;  // for Proxy ARP

  public:
    ARP() {}
    ~ARP();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    void processOutboundPacket(cMessage *msg);
    void sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress);

    void initiateARPResolution(ARPCacheEntry *entry);
    void sendARPRequest(InterfaceEntry *ie, IPAddress ipAddress);
    void requestTimedOut(cMessage *selfmsg);
    bool addressRecognized(IPAddress destAddr, InterfaceEntry *ie);
    void processARPPacket(ARPPacket *arp);
    void updateARPCache(ARPCacheEntry *entry, const MACAddress& macAddress);

    void dumpARPPacket(ARPPacket *arp);
    void updateDisplayString();

};

#endif

