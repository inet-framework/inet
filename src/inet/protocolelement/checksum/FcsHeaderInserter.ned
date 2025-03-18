//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsInserterBase;

//
// Inserts FCS (Frame Check Sequence) headers into packets to enable data
// integrity verification. This module extends FcsInserterBase and adds the
// ability to specify the position of the FCS header (front or back of the
// packet, with back being the default). It processes packets by creating a new
// FCS header, computing the FCS value based on the packet content and FCS mode,
// setting the FCS value and mode in the header, and inserting the header at the
// specified position. It registers with the AccessoryProtocol service for FCS
// insertion.
//
simple FcsHeaderInserter extends FcsInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderInserter);
}
