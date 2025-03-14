//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Calculates and appends the Frame Check Sequence (FCS) to Ethernet frame fragments
// in IEEE 802.3br frame preemption. This module is specifically designed to handle
// the FCS requirements for fragmented frames, ensuring data integrity for each fragment.
//
// When a preemptable frame is interrupted by a higher-priority express frame, it is
// split into fragments. Each fragment requires its own FCS to allow receivers to
// verify fragment integrity independently. This module adds the appropriate FCS to
// each fragment before transmission.
//
// The module supports three operating modes controlled by the fcsMode parameter:
// - disabled: No FCS is added to fragments (used for special simulation scenarios)
// - declared: An FCS field is added, but the actual CRC value is not computed
//   (more efficient for simulations where bit-level errors are not modeled)
// - computed: The FCS is properly calculated based on the fragment contents
//   (more accurate but computationally more intensive)
//
// This module is typically used within the EthernetFragmentingMacLayer as part of
// the frame preemption implementation, working in conjunction with the
// EthernetFragmentFcsChecker that verifies fragment FCS on the receiving end.
//
// @see ~EthernetFragmentingMacLayer, ~EthernetFragmentFcsChecker, ~EthernetPreemptingMacLayer
//
simple EthernetFragmentFcsInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        @class(EthernetFragmentFcsInserter);
        @display("i=block/inserter");
}
