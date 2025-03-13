//
//  LMac.ned
//  LMAC for MF 2.02, omnetpp 4
//
//  Created by Anna Foerster on 10/10/08.
//  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
//
//  Converted to OMNeT++ 4 by Rudolf Hornig
//
//  Converted to MiXiM by Kapourniotis Theodoros
//

package inet.linklayer.lmac;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;


//
// Implements the L-MAC (Lightweight Medium Access Protocol) for wireless sensor
// networks, designed to provide energy-efficient, collision-free communication
// through a time-slotted approach.
//
// Key features of L-MAC include:
// - Time division: The time is divided into frames, and each frame is divided into slots
// - Slot assignment: Each node claims a unique timeslot for transmission
// - Collision avoidance: Nodes detect and resolve slot collisions by selecting new slots
// - Energy efficiency: Nodes sleep during unused slots to conserve energy
//
// The protocol operates as follows:
// - During initialization, nodes select their own unique timeslots
// - In its own timeslot, a node wakes up, performs clear channel assessment (CCA),
//   sends a control packet, and if it has data to send, transmits a data packet
// - In foreign timeslots, a node briefly wakes up to check for control packets
//   addressed to it, and if found, stays awake to receive the data packet
// - If slot collisions are detected, nodes automatically select new available slots
//
// L-MAC is particularly suitable for static wireless sensor networks where
// predictable, energy-efficient communication is required.
//
// The protocol's finite state machine is illustrated in the LMAC-FSM.jpg file.
//
// For more details, see: L. van Hoesel and P. Havinga, "A lightweight medium access
// protocol (LMAC) for wireless sensor networks," IPSN 2004.
//
module LMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address @mutable = default("auto"); // MAC address as hex string (12 hex digits), or
                                                   // "auto". "auto" values will be replaced by
                                                   // a generated MAC address in init stage 0.
        // how long is a slot?
        double slotDuration @unit(s) = default(100ms);
        int headerLength @unit(b) = default(10b);
        int ctrlFrameLength @unit(b) = default(headerLength);
        int mtu @unit(B) = default(0B);
        // default channel
        double defaultChannel = default(0);
        // bit rate
        double bitrate @unit(bps) = default(19200bps);
        // level at which we consider medium busy FIXME not used currently
//        double busyRSSI;

        int reservedMobileSlots = default(2);
        int numSlots = default(64);
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        @class(LMac);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @selfMessageKinds(inet::LMacType);
}
