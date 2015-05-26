/*
 * Copyright (C) 2004 Andras Varga
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

#ifndef __INET_ARP_H
#define __INET_ARP_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/IARP.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

// Forward declarations:
class ARPPacket;
class IInterfaceTable;
class InterfaceEntry;
class IIPv4RoutingTable;

/**
 * ARP implementation.
 */
class INET_API ARP : public cSimpleModule, public IARP, public ILifecycle
{
  public:
    class ARPCacheEntry;
    typedef std::map<IPv4Address, ARPCacheEntry *> ARPCache;
    typedef std::vector<cMessage *> MsgPtrVector;

    // IPv4Address -> MACAddress table
    // TBD should we key it on (IPv4Address, InterfaceEntry*)?
    class ARPCacheEntry
    {
      public:
        ARP *owner = nullptr;    // owner ARP module of this cache entry
        const InterfaceEntry *ie = nullptr;    // NIC to send the packet to
        bool pending = false;    // true if resolution is pending
        MACAddress macAddress;    // MAC address
        simtime_t lastUpdate;    // entries should time out after cacheTimeout
        int numRetries = 0;    // if pending==true: 0 after first ARP request, 1 after second, etc.
        cMessage *timer = nullptr;    // if pending==true: request timeout msg
        ARPCache::iterator myIter;    // iterator pointing to this entry
    };

  protected:
    simtime_t retryTimeout;
    int retryCount = 0;
    simtime_t cacheTimeout;
    bool respondToProxyARP = false;

    bool isUp = false;

    long numResolutions = 0;
    long numFailedResolutions = 0;
    long numRequestsSent = 0;
    long numRepliesSent = 0;

    static simsignal_t sentReqSignal;
    static simsignal_t sentReplySignal;

    ARPCache arpCache;

    IInterfaceTable *ift = nullptr;
    IIPv4RoutingTable *rt = nullptr;    // for answering ProxyARP requests

  protected:
    // Maps an IP multicast address to an Ethernet multicast address.
    MACAddress mapMulticastAddress(IPv4Address addr);

  public:
    ARP();
    virtual ~ARP();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /// IARP implementation  @{
    virtual MACAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) override;
    virtual L3Address getL3AddressFor(const MACAddress& addr) const override;
    /// @}

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void finish() override;

    virtual bool isNodeUp();
    virtual void stop();
    virtual void start();
    virtual void flush();

    virtual void sendPacketToNIC(cMessage *msg, const InterfaceEntry *ie, const MACAddress& macAddress, int etherType);

    virtual void initiateARPResolution(ARPCacheEntry *entry);
    virtual void sendARPRequest(const InterfaceEntry *ie, IPv4Address ipAddress);
    virtual void requestTimedOut(cMessage *selfmsg);
    virtual bool addressRecognized(IPv4Address destAddr, InterfaceEntry *ie);
    virtual void processARPPacket(ARPPacket *arp);
    virtual void updateARPCache(ARPCacheEntry *entry, const MACAddress& macAddress);

    virtual void dumpARPPacket(ARPPacket *arp);
    virtual void updateDisplayString();
};

} // namespace inet

#endif // ifndef __INET_ARP_H

