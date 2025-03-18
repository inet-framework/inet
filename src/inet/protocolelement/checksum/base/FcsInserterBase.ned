//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

//
// Base module for FCS (Frame Check Sequence) inserters that add checksums to
// packets for integrity verification. This module supports multiple FCS modes:
// disabled (0x00000000), declared correct (0xC00DC00D), declared incorrect
// (0xBAADBAAD), and computed. In computed mode, it calculates the actual FCS
// based on the packet content using the Ethernet CRC algorithm. The fcsMode
// parameter determines which mode is used.
//
simple FcsInserterBase extends PacketFlowBase
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        @class(FcsInserterBase);
        @display("i=block/inserter");
}
