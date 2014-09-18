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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_MACADDRESSTABLE_H
#define __INET_MACADDRESSTABLE_H

#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ethernet/switch/IMACAddressTable.h"

namespace inet {

/**
 * This module handles the mapping between ports and MAC addresses. See the NED definition for details.
 */
class MACAddressTable : public cSimpleModule, public IMACAddressTable
{
  protected:
    struct AddressEntry
    {
        unsigned int vid;    // VLAN ID
        int portno;    // Input port
        simtime_t insertionTime;    // Arrival time of Lookup Address Table entry
        AddressEntry() : vid(0) {}
        AddressEntry(unsigned int vid, int portno, simtime_t insertionTime) :
            vid(vid), portno(portno), insertionTime(insertionTime) {}
    };

    friend std::ostream& operator<<(std::ostream& os, const AddressEntry& entry);

    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const { return u1.compareTo(u2) < 0; }
    };

    typedef std::map<MACAddress, AddressEntry, MAC_compare> AddressTable;
    typedef std::map<unsigned int, AddressTable *> VlanAddressTable;

    simtime_t agingTime;    // Max idle time for address table entries
    simtime_t lastPurge;    // Time of the last call of removeAgedEntriesFromAllVlans()
    AddressTable *addressTable;    // VLAN-unaware address lookup (vid = 0)
    VlanAddressTable vlanAddressTable;    // VLAN-aware address lookup

  protected:

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    /**
     * @brief Returns a MAC Address Table for a specified VLAN ID
     */
    AddressTable *getTableForVid(unsigned int vid);

  public:

    MACAddressTable();
    ~MACAddressTable();

  public:
    // Table management

    /**
     * @brief For a known arriving port, V-TAG and destination MAC. It finds out the port where relay component should deliver the message
     * @param address MAC destination
     * @param vid VLAN ID
     * @return Output port for address, or -1 if unknown.
     */
    virtual int getPortForAddress(MACAddress& address, unsigned int vid = 0);

    /**
     * @brief Register a new MAC address at AddressTable.
     * @return True if refreshed. False if it is new.
     */
    virtual bool updateTableWithAddress(int portno, MACAddress& address, unsigned int vid = 0);

    /**
     *  @brief Clears portno cache
     */
    // TODO: find a better name
    virtual void flush(int portno);

    /**
     *  @brief Prints cached data
     */
    virtual void printState();

    /**
     * @brief Copy cache from portA to portB port
     */
    virtual void copyTable(int portA, int portB);

    /**
     * @brief Remove aged entries from a specified VLAN
     */
    virtual void removeAgedEntriesFromVlan(unsigned int vid = 0);
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
    virtual void readAddressTable(const char *fileName);

    /**
     * For lifecycle: clears all entries from the vlanAddressTable.
     */
    virtual void clearTable();

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime);
    virtual void resetDefaultAging();
};

} // namespace inet

#endif // ifndef __INET_MACADDRESSTABLE_H

