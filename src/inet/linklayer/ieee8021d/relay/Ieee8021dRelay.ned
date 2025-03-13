//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.relay;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;


//
// Implements an IEEE 802.1D-compliant bridge relay unit that forwards Ethernet frames
// based on destination MAC addresses while respecting spanning tree protocol states.
// This module extends the basic MAC relay functionality with awareness of spanning
// tree protocols to prevent forwarding loops.
//
// The relay unit operates in two distinct modes:
//
// 1. STP-unaware mode (when no spanning tree protocol module is connected):
//    - Performs standard MAC address learning and forwarding
//    - Looks up destination addresses in the ~MacForwardingTable
//    - Forwards frames to the appropriate port if the destination is known
//    - Broadcasts frames to all ports except the incoming one if the destination is unknown
//    - Learns source MAC addresses and their associated ports
//
// 2. STP-aware mode (when an ~Stp or ~Rstp module is connected):
//    - Processes Bridge Protocol Data Units (BPDUs) for the spanning tree protocol
//    - Forwards BPDUs to/from the spanning tree protocol module
//    - Checks port states in the ~InterfaceTable before forwarding data frames
//    - Only forwards frames through ports in the forwarding state
//    - Blocks forwarding on ports in blocking, listening, or learning states
//    - Maintains MAC address learning even on blocked ports
//
// This module is a key component in implementing Ethernet switches with loop prevention
// capabilities. It works in conjunction with spanning tree protocols to ensure a
// loop-free topology while maintaining connectivity.
//
// @see ~MacForwardingTable, ~Stp, ~Rstp, ~InterfaceTable
//
simple Ieee8021dRelay extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(Ieee8021dRelay);
}
