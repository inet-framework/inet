//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

//
// Base module for CRC (Cyclic Redundancy Check) inserters that add checksums to
// packets for integrity verification. This module supports multiple CRC modes:
// disabled (0x0000), declared correct (0xC00D), declared incorrect (0xBAAD), and
// computed. In computed mode, it calculates the actual CRC based on the packet
// content using the TcpIpChecksum algorithm. The crcMode parameter determines
// which mode is used.
//
simple CrcInserterBase extends PacketFlowBase
{
    parameters:
        string crcMode @enum("disabled","declared","computed") = default("declared");
        @class(CrcInserterBase);
        @display("i=block/inserter");
}
