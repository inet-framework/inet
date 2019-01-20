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

#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * This class provides an IArp implementation whithout exchanging packets.
 */
class INET_API GlobalArp : public OperationalBase, public IArp, public cListener
{
  public:
    class ArpCacheEntry;
    typedef std::map<L3Address, ArpCacheEntry *> ArpCache;

    // L3Address -> MacAddress
    class ArpCacheEntry
    {
      public:
        GlobalArp *owner = nullptr;    // owner module of this cache entry
        const InterfaceEntry *interfaceEntry = nullptr;    // NIC to send the packet to
    };

  protected:
    IInterfaceTable *interfaceTable = nullptr;
    L3Address::AddressType addressType = static_cast<L3Address::AddressType>(-1);

    static ArpCache globalArpCache;
    static int globalArpCacheRefCnt;

  protected:
    void ensureCacheEntry(const L3Address& address, const InterfaceEntry *interfaceEntry);
    MacAddress mapUnicastAddress(L3Address address);
    MacAddress mapMulticastAddress(L3Address address);

  public:
    GlobalArp();
    virtual ~GlobalArp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @name IArp implementation */
    //@{
    virtual L3Address getL3AddressFor(const MacAddress& addr) const override;
    virtual MacAddress resolveL3Address(const L3Address& address, const InterfaceEntry *interfaceEntry) override;
    // @}

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleSelfMessage(cMessage *msg);
    virtual void handlePacket(Packet *packet);

    // Lifecycle methods
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif // ifndef __INET_GLOBALARP_H

