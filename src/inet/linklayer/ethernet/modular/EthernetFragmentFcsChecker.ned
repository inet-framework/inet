//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.checksum.base.ChecksumCheckerBase;
import inet.queueing.contract.IPacketFilter;

simple EthernetFragmentFcsChecker extends ChecksumCheckerBase like IPacketFilter
{
    parameters:
        checksumType = "crc32";
        @class(EthernetFragmentFcsChecker);
        @display("i=block/checker");
}
