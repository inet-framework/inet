/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef _MACRELAYUNITBASE_H
#define _MACRELAYUNITBASE_H

#include <omnetpp.h>
#include <map>
#include <string>
#include "MACAddress.h"

class EtherFrame;


/**
 * Implements base switching functionality of Ethernet switches. Note that
 * neither activity() nor handleMessage() is redefined here -- active
 * behavior (incl. queueing and performance aspects) must be addressed
 * in subclasses.
 */
class MACRelayUnitBase : public cSimpleModule
{
    Module_Class_Members(MACRelayUnitBase,cSimpleModule,0);

  protected:
    // An entry of the Address Lookup Table
    struct AddressEntry
    {
        int portno;             // Input port
        double insertionTime;   // Arrival time of Lookup Address Table entry
    };

    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const
            {return u1.compareTo(u2) < 0;}
    };

    typedef std::map<MACAddress, AddressEntry, MAC_compare> AddressTable;

    // Parameters controlling how the switch operates
    int numPorts;               // Number of ports of the switch
    int addressTableSize;       // Size of the Address Table
    simtime_t agingTime;        // Determines when Ethernet entries are to be removed

    AddressTable addresstable;  // Address Lookup Table

    int seqNum;                 // counter for PAUSE frames

  protected:
    /**
     * Read parameters parameters.
     */
    void initialize();

    /**
     * Updates address table with source address, determines output port
     * and sends out (or broadcasts) frame on ports. Includes calls to
     * updateTableWithAddress() and getPortForAddress().
     *
     * The message pointer should not be referenced any more after this call.
     */
    void handleAndDispatchFrame(EtherFrame *frame, int inputport);

    /**
     * Utility function: sends the frame on all ports except inputport.
     * The message pointer should not be referenced any more after this call.
     */
    void broadcastFrame(EtherFrame *frame, int inputport);

    /**
     * Pre-reads in entries for Address Table during initialization.
     */
    void readAddressTable(const char* fileName);

    /**
     * Enters address into table.
     */
    void updateTableWithAddress(MACAddress& address, int portno);

    /**
     * Returns output port for address, or -1 if unknown.
     */
    int getPortForAddress(MACAddress& address);

    /**
     * Prints contents of address table on ev.
     */
    void printAddressTable();

    /**
     * Utility function: throws out all aged entries from table.
     */
    void removeAgedEntriesFromTable();

    /**
     * Utility function: throws out oldest (not necessarily aged) entry from table.
     */
    void removeOldestTableEntry();

    /**
     * Utility function (for use by subclasses) to send a flow control
     * PAUSE frame on the given port.
     */
    void sendPauseFrame(int portno, int pauseUnits);

};

#endif


