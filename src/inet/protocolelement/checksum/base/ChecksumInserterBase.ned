//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

//
// Base module for checksum inserters that add checksums to packets for integrity
// verification. This module supports multiple checksum algorithms such as CRC32
// and the Internet checksum, and also several checksum modes: disabled,
// declared correct, declared incorrect, and computed. In computed mode,
// it calculates the actual checksum based on the packet content using the selected
// checksum algorithm. The checksumMode parameter determines which mode is used.
//
// @see ChecksumInserterBase
//
simple ChecksumInserterBase extends PacketFlowBase
{
    parameters:
        string checksumType @enum("internet","ethernet-fcs","crc32c","crc16-ibm","crc16-ccitt") = default("ethernet-fcs");
        string checksumMode @enum("disabled","declared","computed") = default("declared");
        @class(ChecksumInserterBase);
        @display("i=block/inserter");
}
