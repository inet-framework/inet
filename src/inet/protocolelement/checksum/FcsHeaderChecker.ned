//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsCheckerBase;

//
// Checks FCS (Frame Check Sequence) values in packet headers to verify data
// integrity. This module extends FcsCheckerBase and adds the ability to specify
// the position of the FCS header (front or back of the packet, with back being
// the default). It processes packets by popping the FCS header, checking if the
// FCS value matches the expected value based on the FCS mode, and filtering out
// packets with invalid checksums. It registers with the AccessoryProtocol
// service for FCS checking.
//
simple FcsHeaderChecker extends FcsCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderChecker);
}
