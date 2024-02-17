//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACFORWARDINGTABLE_H
#define __INET_MACFORWARDINGTABLE_H

#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

/**
 * This module handles the mapping between interface IDs and MAC addresses. See the NED definition for details.
 * NOTE that interfaceIds (interfaceId parameters) are actually the corresponding ID of the port interface.
 * i.e. this is an interfaceId and NOT an index of the some kind in a gate vector.
 */
class INET_API MacForwardingTable : public OperationalBase, public IMacForwardingTable, public StringFormat::IDirectiveResolver
{
  protected:
    struct AddressEntry {
        int interfaceId = -1;
        simtime_t insertionTime;
        AddressEntry() {}
        AddressEntry(unsigned int vid, int interfaceId, simtime_t insertionTime) :
            interfaceId(interfaceId), insertionTime(insertionTime) {}
    };

    struct MulticastAddressEntry {
        std::vector<int> interfaceIds;
        MulticastAddressEntry() {}
        MulticastAddressEntry(unsigned int vid, const std::vector<int>& interfaceIds) :
            interfaceIds(interfaceIds) {}
    };

    friend std::ostream& operator<<(std::ostream& os, const AddressEntry& entry);
    friend std::ostream& operator<<(std::ostream& os, const MulticastAddressEntry& entry);

    struct MacCompare {
        bool operator()(const MacAddress& u1, const MacAddress& u2) const { return u1.compareTo(u2) < 0; }
    };

    typedef std::pair<unsigned int, MacAddress> ForwardingTableKey;
    friend std::ostream& operator<<(std::ostream& os, const ForwardingTableKey& key);
    typedef std::map<ForwardingTableKey, AddressEntry> ForwardingTable;
    typedef std::map<ForwardingTableKey, MulticastAddressEntry> MulticastForwardingTable;

    simtime_t agingTime; // Max idle time for address table entries
    simtime_t lastPurge; // Time of the last call of removeAgedEntriesFromAllVlans()
    ForwardingTable forwardingTable;
    MulticastForwardingTable multicastForwardingTable;
    ModuleRefByPar<IInterfaceTable> ifTable;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual std::string resolveDirective(char directive) const override;

    virtual void parseForwardingTableParameter();

  public:
    // IMacForwardingTable
    virtual int getUnicastAddressForwardingInterface(const MacAddress& address, unsigned int vid = 0) const override;
    virtual void setUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void removeUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void learnUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;

    virtual std::vector<int> getMulticastAddressForwardingInterfaces(const MacAddress& address, unsigned int vid = 0) const override;
    virtual void addMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;
    virtual void removeMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;

    virtual void removeForwardingInterface(int interfaceId) override;
    virtual void replaceForwardingInterface(int oldInterfaceId, int newInterfaceId) override;

  protected:
    /**
     *  @brief Prints cached data
     */
    virtual void printState();

    /**
     * @brief Remove aged entries from all VLANs
     */
    virtual void removeAgedEntriesFromAllVlans();

    /*
     * It calls removeAgedEntriesFromAllVlans() if and only if at least
     * 1 second has passed since the method was last called.
     */
    virtual void removeAgedEntriesIfNeeded();

    /**
     * Pre-reads in entries for Address Table during initialization.
     */
    virtual void readForwardingTable(const char *fileName);

    /**
     * For lifecycle: initialize entries for the vlanForwardingTable by reading them from a file (if specified by a parameter)
     */
    virtual void initializeTable();

    /**
     * For lifecycle: clears all entries from the vlanForwardingTable.
     */
    virtual void clearTable();

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) override;

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { initializeTable(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    //@}
};

} // namespace inet

#endif

