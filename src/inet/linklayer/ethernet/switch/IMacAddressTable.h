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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IMACADDRESSTABLE_H
#define __INET_IMACADDRESSTABLE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

/*
 * A C++ interface to abstract the functionality of IMacAddressTable.
 */
class INET_API IMacAddressTable
{
  public:
    /**
     * @brief For a known arriving port, V-TAG and destination MAC. It finds out the interface Id where relay component should deliver the message
     * @param address MAC destination
     * @param vid VLAN ID
     * @return Output interface Id for address, or -1 if unknown.
     */
    virtual int getInterfaceIdForAddress(const MacAddress& address, unsigned int vid = 0) = 0;

    /**
     * @brief Register a new MAC address at AddressTable.
     * @return True if refreshed. False if it is new.
     */
    virtual bool updateTableWithAddress(int portno, const MacAddress& address, unsigned int vid = 0) = 0;

    /**
     *  @brief Clears portno cache
     */
    virtual void flush(int portno) = 0;

    /**
     *  @brief Prints cached data
     */
    virtual void printState() = 0;

    /**
     * @brief Copy cache from portA to portB port
     */
    virtual void copyTable(int portA, int portB) = 0;

    /**
     * @brief Remove aged entries from a specified VLAN
     */
    virtual void removeAgedEntriesFromVlan(unsigned int vid = 0) = 0;
    /**
     * @brief Remove aged entries from all VLANs
     */
    virtual void removeAgedEntriesFromAllVlans() = 0;

    /*
     * It calls removeAgedEntriesFromAllVlans() if and only if at least
     * 1 second has passed since the method was last called.
     */
    virtual void removeAgedEntriesIfNeeded() = 0;

    /**
     * Pre-reads in entries for Address Table during initialization.
     */
    virtual void readAddressTable(const char *fileName) = 0;

    /**
     * For lifecycle: initialize entries for the vlanAddressTable by reading them from a file (if specified by a parameter)
     */
    virtual void initializeTable() = 0;

    /**
     * For lifecycle: clears all entries from the vlanAddressTable.
     */
    virtual void clearTable() = 0;

    /*
     * Some (eg.: STP, RSTP) protocols may need to change agingTime
     */
    virtual void setAgingTime(simtime_t agingTime) = 0;
    virtual void resetDefaultAging() = 0;
};

} // namespace inet

#endif // ifndef __INET_IMACADDRESSTABLE_H

