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
        string checksumType @enum("internet","ethernet-fcs","crc32c","crc16-ibm","crc16-ccitt") = default("ethernet-fcs");
        @class(ChecksumCheckerBase);
        @display("i=block/checker");
}
