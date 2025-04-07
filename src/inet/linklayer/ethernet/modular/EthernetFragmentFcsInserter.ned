//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.checksum.base.ChecksumInserterBase;
import inet.queueing.contract.IPacketFlow;

simple EthernetFragmentFcsInserter extends ChecksumInserterBase like IPacketFlow
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        checksumMode = fcsMode;
        checksumType = "crc32";
        @class(EthernetFragmentFcsInserter);
        @display("i=block/inserter");
}
