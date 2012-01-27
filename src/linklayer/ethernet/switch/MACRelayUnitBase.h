/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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


#ifndef __INET_MACRELAYUNITBASE_H
#define __INET_MACRELAYUNITBASE_H

#include <map>
#include <string>

#include "INETDefs.h"

#include "MACAddress.h"

class EtherFrame;


/**
 * Implements base switching functionality of Ethernet switches. Note that
 * neither activity() nor handleMessage() is redefined here -- active
 * behavior (incl. queueing and performance aspects) must be addressed
 * in subclasses.
 */
class INET_API MACRelayUnitBase : public cSimpleModule
{
  public:
    // An entry of the Address Lookup Table
    struct AddressEntry
    {
        int portno;              // Input port
        simtime_t insertionTime; // Arrival time of Lookup Address Table entry
    };

  protected:
    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const
            {return u1.compareTo(u2) < 0;}
    };

    typedef std::map<MACAddress, AddressEntry, MAC_compare> AddressTable;

    // Parameters controlling how the switch operates
    int numPorts;               // Number of ports of the switch
    int addressTableSize;       // Maximum size of the Address Table
    simtime_t agingTime;        // Determines when Ethernet entries are to be removed

    AddressTable addresstable;  // Address Lookup Table

    int seqNum;                 // counter for PAUSE frames
    simtime_t *pauseFinished;   // finish time of last PAUSE (array of numPorts element)

  public:
    MACRelayUnitBase() { pauseFinished = NULL; }
    ~MACRelayUnitBase() { delete [] pauseFinished; }

  protected:
    /**
     * Read parameters parameters.
     */
    virtual void initialize();

    /**
     * Updates address table with source address, determines output port
     * and sends out (or broadcasts) frame on ports. Includes calls to
     * updateTableWithAddress() and getPortForAddress().
     *
     * The message pointer should not be referenced any more after this call.
     */
    virtual void handleAndDispatchFrame(EtherFrame *frame, int inputport);

    /**
     * Utility function: sends the frame on all ports except inputport.
     * The message pointer should not be referenced any more after this call.
     */
    virtual void broadcastFrame(EtherFrame *frame, int inputport);

    /**
     * Pre-reads in entries for Address Table during initialization.
     */
    virtual void readAddressTable(const char* fileName);

    /**
     * Enters address into table.
     */
    virtual void updateTableWithAddress(MACAddress& address, int portno);

    /**
     * Returns output port for address, or -1 if unknown.
     */
    virtual int getPortForAddress(MACAddress& address);

    /**
     * Prints contents of address table on ev.
     */
    virtual void printAddressTable();

    /**
     * Utility function: throws out all aged entries from table.
     */
    virtual void removeAgedEntriesFromTable();

    /**
     * Utility function: throws out oldest (not necessarily aged) entry from table.
     */
    virtual void removeOldestTableEntry();

    /**
     * Utility function (for use by subclasses) to send a flow control
     * PAUSE frame on the given port.
     */
    virtual void sendPauseFrame(int portno, int pauseUnits);

    /**
     * Utility function (for use by subclasses) to send flow control
     * PAUSE frame on the ports, which previous PAUSE time is terminated.
     */
    virtual void sendPauseFramesIfNeeded(int pauseUnits);
};

#endif

