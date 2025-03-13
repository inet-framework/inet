//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Calculates and appends the Frame Check Sequence (FCS) to outgoing Ethernet frames.
// This module is a critical component of the Ethernet MAC layer's error detection
// mechanism, working in conjunction with the EthernetFcsChecker.
//
// The FCS is a 32-bit CRC (Cyclic Redundancy Check) value appended to the end of
// each Ethernet frame. It is calculated based on all preceding bytes in the frame
// and allows receivers to detect if any bits were corrupted during transmission.
//
// The module supports three operating modes controlled by the fcsMode parameter:
// - disabled: No FCS is added to the frame (used for special simulation scenarios)
// - declared: An FCS field is added, but the actual CRC value is not computed
//   (more efficient for simulations where bit-level errors are not modeled)
// - computed: The FCS is properly calculated based on the frame contents
//   (more accurate but computationally more intensive)
//
// In real Ethernet networks, the FCS is always computed to ensure data integrity.
// The "declared" mode is a simulation optimization that maintains correct frame
// sizes without the computational overhead of CRC calculation when bit-level
// errors are not being simulated.
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline, paired with the EthernetFcsChecker module that
// verifies the FCS on incoming frames.
//
// @see ~EthernetMacLayer, ~EthernetFcsChecker, ~LayeredEthernetInterface
//
simple EthernetFcsInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");  // Controls how the FCS is handled
        @class(EthernetFcsInserter);
        @display("i=block/inserter");
}
