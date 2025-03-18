//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.CrcInserterBase;

//
// Inserts CRC (Cyclic Redundancy Check) headers into packets to enable data
// integrity verification. This module extends CrcInserterBase and adds the
// ability to specify the position of the CRC header (front or back of the
// packet). It processes packets by creating a new CRC header, computing the
// CRC value based on the packet content and CRC mode, setting the CRC value
// and mode in the header, and inserting the header at the specified position.
// It registers with the AccessoryProtocol service for CRC insertion.
//
simple CrcHeaderInserter extends CrcInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(CrcHeaderInserter);
}
