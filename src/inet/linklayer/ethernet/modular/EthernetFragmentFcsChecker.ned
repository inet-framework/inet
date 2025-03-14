//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Verifies the Frame Check Sequence (FCS) of Ethernet frame fragments in
// IEEE 802.3br frame preemption. This module is specifically designed to
// detect transmission errors in fragmented frames by validating each
// fragment's FCS independently.
//
// When a preemptable frame is interrupted by a higher-priority express frame,
// it is split into fragments. Each fragment has its own FCS to allow receivers
// to verify fragment integrity independently. This module checks the FCS of
// each incoming fragment and discards fragments with invalid FCS values.
//
// The module performs the following operations:
// - Recalculates the FCS for each incoming fragment
// - Compares the calculated FCS with the received FCS value
// - Discards fragments with invalid FCS (simulating error detection)
// - Forwards valid fragments for reassembly
//
// This module is typically used within the EthernetFragmentingMacLayer as part
// of the frame preemption implementation, working in conjunction with the
// EthernetFragmentFcsInserter that adds FCS to outgoing fragments.
//
// @see ~EthernetFragmentingMacLayer, ~EthernetFragmentFcsInserter, ~EthernetPreemptingMacLayer
//
simple EthernetFragmentFcsChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetFragmentFcsChecker);
        @display("i=block/checker");
}
