//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_MACADDRESSTABLE_H
#define __INET_MACADDRESSTABLE_H

#include "inet/common/StringFormat.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/contract/IMacAddressTable.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

/**
 * This module handles the mapping between interface IDs and MAC addresses. See the NED definition for details.
 * NOTE that interfaceIds (interfaceId parameters) are actually the corresponding ID of the port interface.
 * i.e. this is an interfaceId and NOT an index of the some kind in a gate vector.
 */
class INET_API MacAddressTable : public OperationalBase, public IMacAddressTable, public StringFormat::IDirectiveResolver
{
  protected:
    struct AddressEntry {
        unsigned int vid = 0; // VLAN ID
        std::vector<int> interfaceIds; // network interface IDs
        simtime_t insertionTime; // Arrival time of Lookup Address Table entry
        AddressEntry() {}
        AddressEntry(unsigned int vid, const std::vector<int>& interfaceIds, simtime_t insertionTime) :
            vid(vid), interfaceIds(interfaceIds), insertionTime(insertionTime) {}
    };

    friend std::ostream& operator<<(std::ostream& os, const AddressEntry& entry);

    typedef std::pair<unsigned int, MacAddress> AddressTableKey;
    friend std::ostream& operator<<(std::ostream& os, const AddressTableKey& key);
    typedef std::map<AddressTableKey, AddressEntry> AddressTable;

    simtime_t agingTime; // Max idle time for address table entries
    simtime_t lastPurge; // Time of the last call of removeAgedEntriesFromAllVlans()
    AddressTable addressTable;
    ModuleRefByPar<IInterfaceTable> ifTable;

  protected:

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual const char *resolveDirective(char directive) const override;

    virtual void parseAddressTableParameter();
    virtual bool updateMacAddressTable(int interfaceId, const MacAddress& address, unsigned int vid);

  public:

    MacAddressTable();
    ~MacAddressTable();

  public:
    // Table management

    /**
     * @brief For a given port, V-TAG and destination MAC it finds out the interface Id where relay component should deliver the message.
     * @param address MAC destination
     * @param vid VLAN ID
     * @return Output interface Id for address, or -1 if unknown.
     */
    virtual int getInterfaceIdForAddress(const MacAddress& address, unsigned int vid = 0) override;

    /**
     * @brief For a given port, V-TAG and destination MAC it finds out the interface Ids where relay component should deliver the message.
     * @param address MAC destination
     * @param vid VLAN ID
     * @return Output interface Ids for address, or empty list unknown.
     */
    virtual std::vector<int> getInterfaceIdsForAddress(const MacAddress& address, unsigned int vid = 0) override;

    /**
     * @brief Register a new MAC address at AddressTable.
     * @return True if refreshed. False if it is new.
     */
    virtual bool learnMacAddress(int interfaceId, const MacAddress& address, unsigned int vid = 0) override;

    /**
     *  @brief Clears interfaceId cache
     */
    // TODO find a better name
    virtual void flush(int interfaceId) override;

    /**
     *  @brief Prints cached data
     */
    virtual void printState() override;

    /**
     * @brief Copy cache from interfaceIdA to interfaceIdB interface
     */
    virtual void copyTable(int interfaceIdA, int interfaceIdB) override;

    /**
     * @brief Remove aged entries from all VLANs
     */
    virtual void removeAgedEntriesFromAllVlans() override;

    /*
     * It calls removeAgedEntriesFromAllVlans() if and only if at least
     * 1 second has passed since the method was last called.
     */
    virtual void removeAgedEntriesIfNeeded() override;

    /**
     * Pre-reads in entries for Address Table during initialization.
     */
    virtual void readAddressTable(const char *fileName) override;

    /**
     * For lifecycle: initialize entries for the vlanAddressTable by reading them from a file (if specified by a parameter)
     */
    virtual void initializeTable() override;

    /**
     * For lifecycle: clears all entries from the vlanAddressTable.
     */
    virtual void clearTable() override;

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) override;
    virtual void resetDefaultAging() override;

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override { initializeTable(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override { clearTable(); }
    virtual bool isInitializeStage(int stage) override { return stage == INITSTAGE_LINK_LAYER; }
    virtual bool isModuleStartStage(int stage) override { return stage == ModuleStartOperation::STAGE_LINK_LAYER; }
    virtual bool isModuleStopStage(int stage) override { return stage == ModuleStopOperation::STAGE_LINK_LAYER; }
    //@}
};

} // namespace inet

#endif

