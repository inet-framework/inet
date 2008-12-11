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

#ifndef __INET_ARP_H
#define __INET_ARP_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <omnetpp.h>
#include "IPAddress.h"
#include "ARPPacket_m.h"
#include "IPControlInfo.h"
#include "IPDatagram.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IRoutingTable.h"
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
    int nicOutBaseGateId;  // id of the nicOut[0] gate

    IInterfaceTable *ift;
    IRoutingTable *rt;  // for Proxy ARP

  public:
    ARP() {}
    virtual ~ARP();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void processOutboundPacket(cMessage *msg);
    virtual void sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& macAddress);

    virtual void initiateARPResolution(ARPCacheEntry *entry);
    virtual void sendARPRequest(InterfaceEntry *ie, IPAddress ipAddress);
    virtual void requestTimedOut(cMessage *selfmsg);
    virtual bool addressRecognized(IPAddress destAddr, InterfaceEntry *ie);
    virtual void processARPPacket(ARPPacket *arp);
    virtual void updateARPCache(ARPCacheEntry *entry, const MACAddress& macAddress);

    virtual void dumpARPPacket(ARPPacket *arp);
    virtual void updateDisplayString();

};

#endif

