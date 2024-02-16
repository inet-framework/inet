// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later


#ifndef __INET_MRPMACFORWARDINGTABLE_H
#define __INET_MRPMACFORWARDINGTABLE_H

#include "inet/linklayer/ethernet/common/MacForwardingTable.h"


namespace inet {

/**
 * This module handles the mapping between interface IDs and MAC addresses. See the NED definition for details.
 * NOTE that interfaceIds (interfaceId parameters) are actually the corresponding ID of the port interface.
 * i.e. this is an interfaceId and NOT an index of the some kind in a gate vector.
 */
class INET_API MrpMacForwardingTable : public MacForwardingTable
{
  protected:
    friend std::ostream& operator<<(std::ostream& os, const AddressEntry& entry);
    friend std::ostream& operator<<(std::ostream& os, const MulticastAddressEntry& entry);
    friend std::ostream& operator<<(std::ostream& os, const ForwardingTableKey& key);
    MulticastForwardingTable mrpForwardingTable;
    MulticastForwardingTable mrpIngressFilterTable;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual std::vector<int> getMrpForwardingInterfaces(const MacAddress& address, unsigned int vid = 0) const;
    virtual void addMrpForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0);
    virtual void removeMrpForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0);
    virtual bool isMrpIngressFilterInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) const;
    virtual void addMrpIngressFilterInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0);
    virtual void removeMrpIngressFilterInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0);
    virtual void clearTable() override;
    virtual void clearMrpTable();

  protected:

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { initializeTable(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    //@}
};

} // namespace inet

#endif

