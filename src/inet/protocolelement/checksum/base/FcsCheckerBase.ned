//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// Base module for FCS (Frame Check Sequence) checkers that verify the integrity
// of packets. This module filters packets based on their FCS value, dropping
// packets with invalid checksums. It supports multiple FCS modes: disabled
// (0x00000000), declared correct (0xC00DC00D), declared incorrect (0xBAADBAAD),
// and computed. When in computed mode, it calculates the actual FCS using the
// Ethernet CRC algorithm and compares it with the received value.
//
simple FcsCheckerBase extends PacketFilterBase
{
    parameters:
        @class(FcsCheckerBase);
        @display("i=block/checker");
}
