//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderInserter;

//
// Inserts Ethernet FCS checksums as headers into packets to enable data
// integrity verification. This module extends ChecksumHeaderInserter and
// specializes it to use the "ethernet-fcs" checksum type.
//
// @see EthernetFcsHeaderChecker
//
simple EthernetFcsHeaderInserter extends ChecksumHeaderInserter
{
    parameters:
        checksumType = "ethernet-fcs";
}
