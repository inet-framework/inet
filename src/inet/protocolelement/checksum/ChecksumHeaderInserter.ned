//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.ChecksumInserterBase;

//
// Adds a checksum to packets to verify data integrity. This module extends 
// ChecksumInserterBase and adds the ability to specify the position of the 
// checksum header (front or back of the packet). It processes packets by
// creating a new checksum header, computing the checksum value based on the
// packet content and checksum mode, setting the checksum value
// and mode in the header, and inserting the header at the specified position.
//
// @see ChecksumHeaderChecker
//
simple ChecksumHeaderInserter extends ChecksumInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(ChecksumHeaderInserter);
}
