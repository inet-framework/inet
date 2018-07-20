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

#include "inet/networklayer/contract/IArp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

// Forward declarations:
class ArpPacket;
class IInterfaceTable;
class InterfaceEntry;
class IIpv4RoutingTable;

/**
 * ARP implementation.
 */
class INET_API Arp : public cSimpleModule, public IArp, public ILifecycle
{
  public:
    class ArpCacheEntry;
    typedef std::map<Ipv4Address, ArpCacheEntry *> ArpCache;
    typedef std::vector<cMessage *> MsgPtrVector;

    // Ipv4Address -> MacAddress table
    // TBD should we key it on (Ipv4Address, InterfaceEntry*)?
    class ArpCacheEntry
    {
      public:
        Arp *owner = nullptr;    // owner ARP module of this cache entry
        const InterfaceEntry *ie = nullptr;    // NIC to send the packet to
        bool pending = false;    // true if resolution is pending
        MacAddress macAddress;    // MAC address
        simtime_t lastUpdate;    // entries should time out after cacheTimeout
        int numRetries = 0;    // if pending==true: 0 after first ARP request, 1 after second, etc.
        cMessage *timer = nullptr;    // if pending==true: request timeout msg
        ArpCache::iterator myIter;    // iterator pointing to this entry
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

    static simsignal_t arpRequestSentSignal;
    static simsignal_t arpReplySentSignal;

    ArpCache arpCache;

    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;    // for answering ProxyARP requests

  protected:
    // Maps an IP multicast address to an Ethernet multicast address.
    MacAddress mapMulticastAddress(Ipv4Address addr);

  public:
    Arp();
    virtual ~Arp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /// IArp implementation  @{
    virtual MacAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) override;
    virtual L3Address getL3AddressFor(const MacAddress& addr) const override;
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

    virtual void initiateARPResolution(ArpCacheEntry *entry);
    virtual void sendARPRequest(const InterfaceEntry *ie, Ipv4Address ipAddress);
    virtual void requestTimedOut(cMessage *selfmsg);
    virtual bool addressRecognized(Ipv4Address destAddr, InterfaceEntry *ie);
    virtual void processARPPacket(Packet *packet);
    virtual void updateARPCache(ArpCacheEntry *entry, const MacAddress& macAddress);

    virtual void dumpARPPacket(const ArpPacket *arp);
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif // ifndef __INET_ARP_H

