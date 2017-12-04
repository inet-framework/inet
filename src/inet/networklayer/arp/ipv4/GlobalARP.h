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

#ifndef __INET_GLOBALARP_H
#define __INET_GLOBALARP_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

// Forward declarations:
class ArpPacket;
class IInterfaceTable;
class InterfaceEntry;
class IIpv4RoutingTable;

/**
 * ARP implementation.
 */
class INET_API GlobalArp : public cSimpleModule, public IArp, public ILifecycle, public cListener
{
  public:
    class ArpCacheEntry;
    typedef std::map<L3Address, ArpCacheEntry *> ArpCache;
    typedef std::vector<cMessage *> MsgPtrVector;

    // Ipv4Address -> MacAddress table
    // TBD should we key it on (Ipv4Address, InterfaceEntry*)?
    class ArpCacheEntry
    {
      public:
        GlobalArp *owner = nullptr;    // owner ARP module of this cache entry
        const InterfaceEntry *ie = nullptr;    // NIC to send the packet to
        MacAddress macAddress;    // MAC address
        simtime_t lastUpdate;    // entries should time out after cacheTimeout
        ArpCache::iterator myIter;    // iterator pointing to this entry
    };

  protected:
    bool isUp = false;

    static ArpCache globalArpCache;
    static int globalArpCacheRefCnt;

    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;    // for answering ProxyARP requests

  protected:
    // Maps an IP multicast address to an Ethernet multicast address.
    MacAddress mapMulticastAddress(L3Address addr);

  public:
    GlobalArp();
    virtual ~GlobalArp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /// IARPCache implementation  @{
    virtual L3Address getL3AddressFor(const MacAddress& addr) const override;
    virtual MacAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) override;
    /// @}

    // cListener
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void finish() override;

    virtual void processSelfMessage(cMessage *msg);
    virtual void handleMessageWhenDown(cMessage *msg);

    virtual bool isNodeUp();
    virtual void stop();
    virtual void start();

    virtual void processARPPacket(ArpPacket *arp);
};

} // namespace inet

#endif // ifndef __INET_GLOBALARP_H

