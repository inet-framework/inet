//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// Base module for CRC (Cyclic Redundancy Check) checkers that verify the
// integrity of packets. This module filters packets based on their CRC value,
// dropping packets with invalid checksums. It supports multiple CRC modes:
// disabled, declared correct, declared incorrect, and computed. When in computed
// mode, it calculates the actual CRC and compares it with the received value.
//
simple CrcCheckerBase extends PacketFilterBase
{
    parameters:
        @class(CrcCheckerBase);
        @display("i=block/checker");
}
