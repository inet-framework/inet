//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.CrcCheckerBase;

//
// Checks CRC (Cyclic Redundancy Check) values in packet headers to verify data
// integrity. This module extends CrcCheckerBase and adds the ability to specify
// the position of the CRC header (front or back of the packet). It processes
// packets by popping the CRC header, checking if the CRC value matches the
// expected value based on the CRC mode, and filtering out packets with invalid
// checksums. It registers with the AccessoryProtocol service for CRC checking.
//
simple CrcHeaderChecker extends CrcCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(CrcHeaderChecker);
}
