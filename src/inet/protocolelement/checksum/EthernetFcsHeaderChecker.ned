//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderChecker;

//
// Checks Ethernet FCS checksums inserted as headers in packets
// in order to verify data integrity. This module extends
// ChecksumHeaderChecker and specializes it to use the "ethernet-fcs"
// checksum type.
//
// @see EthernetFcsHeaderInserter
//
simple EthernetFcsHeaderChecker extends ChecksumHeaderChecker
{
    parameters:
        checksumType = "ethernet-fcs";
}
