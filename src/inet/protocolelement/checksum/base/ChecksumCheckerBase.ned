//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// Base module for checksum checkers that verify the integrity of packets.
// This module filters packets based on their checksum value, dropping packets
// with invalid checksums. It supports multiple checksum algorithms
// such as CRC32 and the Internet checksum, and also several checksum modes:
// disabled, declared correct, declared incorrect, and computed. When in computed
// mode, it calculates the actual checksum and compares it with the received value.
//
// @see ChecksumInserterBase
//
simple ChecksumCheckerBase extends PacketFilterBase
{
    parameters:
        string checksumType @enum("internet","ethernet-fcs","crc32c","crc16-ibm","crc16-ccitt") = default("ethernet-fcs");
        @class(ChecksumCheckerBase);
        @display("i=block/checker");
}
