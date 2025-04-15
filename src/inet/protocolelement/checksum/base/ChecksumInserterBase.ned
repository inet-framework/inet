//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

simple ChecksumInserterBase extends PacketFlowBase
{
    parameters:
        string checksumType @enum("internet","crc32-eth","crc32c","crc16-ibm","crc16-ccitt") = default("crc32-eth"); // note: others are not currently supported
        string checksumMode @enum("disabled","declared","computed") = default("declared");
        @class(ChecksumInserterBase);
        @display("i=block/inserter");
}
