//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

simple ChecksumCheckerBase extends PacketFilterBase
{
    parameters:
        string checksumType @enum("internet","crc32") = default("crc32"); // note: others are not currently supported
        @class(ChecksumCheckerBase);
        @display("i=block/checker");
}
