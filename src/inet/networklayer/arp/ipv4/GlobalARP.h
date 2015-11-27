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
class INET_API GlobalARP : public cSimpleModule, public IARP, public ILifecycle, public cListener
{
  public:
    class ARPCacheEntry;
    typedef std::map<L3Address, ARPCacheEntry *> ARPCache;
    typedef std::vector<cMessage *> MsgPtrVector;

    // IPv4Address -> MACAddress table
    // TBD should we key it on (IPv4Address, InterfaceEntry*)?
    class ARPCacheEntry
    {
      public:
        GlobalARP *owner = nullptr;    // owner ARP module of this cache entry
        const InterfaceEntry *ie = nullptr;    // NIC to send the packet to
        MACAddress macAddress;    // MAC address
        simtime_t lastUpdate;    // entries should time out after cacheTimeout
        ARPCache::iterator myIter;    // iterator pointing to this entry
    };

  protected:
    bool isUp = false;

    static ARPCache globalArpCache;
    static int globalArpCacheRefCnt;

    IInterfaceTable *ift = nullptr;
    IIPv4RoutingTable *rt = nullptr;    // for answering ProxyARP requests

  protected:
    // Maps an IP multicast address to an Ethernet multicast address.
    MACAddress mapMulticastAddress(L3Address addr);

  public:
    GlobalARP();
    virtual ~GlobalARP();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /// IARPCache implementation  @{
    virtual L3Address getL3AddressFor(const MACAddress& addr) const override;
    virtual MACAddress resolveL3Address(const L3Address& address, const InterfaceEntry *ie) override;
    /// @}

    // cListener
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

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

    virtual void processARPPacket(ARPPacket *arp);

    virtual void updateDisplayString();
};

} // namespace inet

#endif // ifndef __INET_GLOBALARP_H

