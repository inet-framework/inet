//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Verifies the Frame Check Sequence (FCS) of incoming Ethernet frames to detect
// transmission errors. This module is a critical component of the Ethernet MAC
// layer's error detection mechanism.
//
// The FCS is a 32-bit CRC (Cyclic Redundancy Check) value appended to the end of
// each Ethernet frame. It is calculated based on all preceding bytes in the frame
// and allows receivers to detect if any bits were corrupted during transmission.
// The module performs the following operations:
//
// - Recalculates the FCS for each incoming frame
// - Compares the calculated FCS with the received FCS value
// - Discards frames with invalid FCS (simulating error detection)
// - Optionally removes the FCS field from valid frames before forwarding them
//
// When the popFcs parameter is set to true (the default), the module removes the
// FCS field from valid frames before passing them to higher layers. This simulates
// the behavior of real Ethernet controllers that typically strip the FCS before
// delivering the frame to the host.
//
// This module is typically used within modular Ethernet interfaces as part of the
// MAC layer processing pipeline. It works in conjunction with the EthernetFcsInserter
// module, which calculates and appends the FCS to outgoing frames.
//
// @see ~EthernetMacLayer, ~EthernetFcsInserter, ~LayeredEthernetInterface
//
simple EthernetFcsChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        bool popFcs = default(true);  // When true, removes the FCS field from valid frames before forwarding
        @class(EthernetFcsChecker);
        @display("i=block/checker");
}
