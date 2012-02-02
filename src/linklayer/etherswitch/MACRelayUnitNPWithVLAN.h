/*
 * Copyright (C) 2012-2011 Kyeong Soo (Joseph) Kim
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

#ifndef __INET_MACRELAYUNITNPWITHVLAN_H
#define __INET_MACRELAYUNITNPWITHVLAN_H


#include <vector>
#include "MACRelayUnitNP.h"
#include "VLAN.h"


//class EtherFrame;
class EthernetIIFrameWithVLAN;

/**
 * Implementation of MACRelayUnitNP with VLAN tags based IEEE 802.1Q.
 */
class INET_API MACRelayUnitNPWithVLAN : public MACRelayUnitNP
{
  public:
//    MACRelayUnitNPWithVLAN();
//    virtual ~MACRelayUnitNPWithVLAN();

  protected:
    // An entry of the VLAN-augmented Address Lookup Table
    struct VLANAddressEntry
    {
        VID vid;                 // VLAN ID (VID)
        int portno;              // Input port
        simtime_t insertionTime; // Arrival time of Lookup Address Table entry
    };

  protected:
    struct MAC_compare
    {
        bool operator()(const MACAddress& u1, const MACAddress& u2) const
            {return u1.compareTo(u2) < 0;}
    };

    typedef std::multimap<MACAddress, VLANAddressEntry, MAC_compare> VLANAddressTable;

    VLANAddressTable addresstable;  // Address Lookup Table augmented with VLAN IDs
    // This redefines 'addresstable' from the base class.
    // Note that the original 'addresstable' from the base class 'MACRelayUnitNP'
    // is still accessible via 'MACRelayUnitNP::addresstable'.

  protected:
    enum  PortRegistration {Fixed, Forbidden, Normal};

    struct PortStatus
    {
        int portno; // port index
        PortRegistration registration;
        bool tagged;    // indicating whether frames are to be VLAN-tagged or untagged when transmitted
    };

    typedef std::vector<PortStatus *> PortMap;    // including a control element for each outbound port

    typedef std::map<VID, PortMap> VLANRegistrationTable;

    VLANRegistrationTable vlanTable;    // VLAN registration table

    // the shared queue
//    cQueue queue;

    // Parameters controlling how the switch operates
//    int numCPUs;                // number of processors
//    simtime_t processingTime;   // Time taken to switch and process a frame
//    int bufferSize;             // Max size of the buffer
//    long highWatermark;         // if buffer goes above this level, send PAUSE frames
//    int pauseUnits;             // "units" field in PAUSE frames
//    simtime_t pauseInterval;    // min time between sending PAUSE frames

    // Other variables
//    int bufferUsed;             // Amount of buffer used to store frames
//    cMessage **endProcEvents;   // self-messages, one for each processor
//    simtime_t pauseLastSent;

    // Parameters for statistics collection
//    long numProcessedFrames;
//    long numDroppedFrames;
//    cOutVector bufferLevel;

  protected:
    /** @name Redefined MACRelayUnitNP member functions. */
    //@{
    /**
     * For multi-stage initialization
     */
    virtual void initialize(int stage);

    /**
     * For multi-stage initialization
     */
    virtual int numInitStages() const {return 2;}

//    /**
//     * Calls handleIncomingFrame() for frames arrived from outside,
//     * and processFrame() for self messages.
//     */
//    virtual void handleMessage(cMessage *msg);

//    /**
//     * Writes statistics.
//     */
//    virtual void finish();
    //@}

//    /**
//     * Handle incoming Ethernet frame with VLAN tag: if buffer full discard it,
//     * otherwise, insert it into buffer and start processing if a processor is free.
//     */
//    virtual void handleIncomingFrame(EthernetIIFrameWithVLAN *msg);

//    /**
//     * Triggered when a frame has completed processing, it routes the frame
//     * to the appropriate port, and starts processing the next frame.
//     */
//    virtual void processFrame(cMessage *msg);

    /**
     *---------------------------------------------------------------------------
     * Redefine virtual member functions of the original base class
     * 'MACRelayUnitBase' to augment bridge learning and relaying operations
     * with VLAN IDs.
     *---------------------------------------------------------------------------
     */
    /** @name Redefined MACRelayUnitBase member functions. */
    //@{
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
    * Pre-reads in entries for VLAN-augmented Address Table during initialization.
    */
   virtual void readAddressTable(const char* fileName);

//    /**
//     * Enters address into table.
//     */
//    virtual void updateTableWithAddress(MACAddress& address, VID vid, int portno);

//    /**
//     * Returns output port for address, or -1 if unknown.
//     */
//    virtual int getPortForAddress(MACAddress& address);

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
//
//    /**
//     * Utility function (for use by subclasses) to send a flow control
//     * PAUSE frame on the given port.
//     */
//    virtual void sendPauseFrame(int portno, int pauseUnits);
   //@}

   /**
    *---------------------------------------------------------------------------
    * Define new virtual member functions to replace the relevant one of the parent
    * classes to provide bridge learning and relaying operations with VLAN IDs.
    *---------------------------------------------------------------------------
    */
   /** @name Newly defined virtual member functions for internal use. */
   //@{
   /**
    * Returns output port for address, or -1 if unknown.
    */
   virtual int getPortForVLANAddress(MACAddress& address, VID vid);

   /**
    * Enters address into VLAN address table (i.e., the 'Filtering Database' in the IEEE 802.1Q standard).
    */
   virtual void updateVLANTableWithAddress(MACAddress& address, VID vid, int portno);
   //@}

   /**
    *---------------------------------------------------------------------------
    * Define new virtual member functions to provide services to other modules.
    *---------------------------------------------------------------------------
    */
  public:
   /** @name Newly defined virtual member functions for other modules. */
   //@{
   /**
    * Returns vid for address, or -1 if unknown.
    */
   virtual int getVIDForMACAddress(MACAddress address);
   //@}
};

#endif
