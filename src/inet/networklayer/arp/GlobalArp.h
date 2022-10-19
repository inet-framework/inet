//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GLOBALARP_H
#define __INET_GLOBALARP_H

#include <map>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IArp.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * This class provides an IArp implementation whithout exchanging packets.
 */
class INET_API GlobalArp : public OperationalBase, public IArp, public cListener
{
  public:
    // L3Address -> MacAddress
    class INET_API ArpCacheEntry {
      public:
        GlobalArp *owner = nullptr; // owner module of this cache entry
        const NetworkInterface *networkInterface = nullptr; // NIC to send the packet to
    };

  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    L3Address::AddressType addressType = static_cast<L3Address::AddressType>(-1);

    typedef std::map<L3Address, ArpCacheEntry *> ArpCache;
    ArpCache& globalArpCache = SIMULATION_SHARED_VARIABLE(globalArpCache);

  protected:
    void ensureCacheEntry(const L3Address& address, const NetworkInterface *networkInterface);
    MacAddress mapUnicastAddress(L3Address address);

  public:
    GlobalArp();
    virtual ~GlobalArp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    static MacAddress toMulticastMacAddress(Ipv4Address address);
    static MacAddress toMulticastMacAddress(Ipv6Address address);

    /** @name IArp implementation */
    //@{
    virtual L3Address getL3AddressFor(const MacAddress& addr) const override;
    virtual MacAddress resolveL3Address(const L3Address& address, const NetworkInterface *networkInterface) override;
    //@}

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleSelfMessage(cMessage *msg);
    virtual void handlePacket(Packet *packet);

    // Lifecycle methods
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

