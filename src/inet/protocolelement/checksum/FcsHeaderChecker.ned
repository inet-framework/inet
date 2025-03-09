//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsCheckerBase;

//
// Verifies Frame Check Sequence (FCS) values in packet headers and
// removes the FCS header if valid. Filters out packets with invalid FCS.
//
// Operation:
// - Examines FCS headers at specified position (typically at the back)
// - Verifies 32-bit FCS values against packet content
// - Removes FCS headers from valid packets
// - Drops packets with invalid FCS values
// - Registers with the AccessoryProtocol service
//
// Particularly useful for:
// - Implementing data link layer protocol verification
// - Adding frame integrity checking to custom protocols
// - Simulating realistic error detection in Ethernet and similar protocols
// - Building protocol stacks with trailer-based checksums
// - Testing protocol robustness against transmission errors
//
// @see ~FcsHeaderInserter, ~CrcHeaderChecker, ~FcsCheckerBase
//
simple FcsHeaderChecker extends FcsCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderChecker);
}
