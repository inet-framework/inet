//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.checksum.base.ChecksumCheckerBase;
import inet.queueing.contract.IPacketFilter;

//
// Verifies the Frame Check Sequence (FCS) in Ethernet frame fragments to detect
// transmission errors. Filters out fragments with incorrect FCS and maintains
// state between fragments to verify the complete frame's FCS. Updates fragment
// tags to indicate if a fragment is the last one based on the mCRC field and
// removes the FCS from fragments that pass the check.
//
simple EthernetFragmentFcsChecker extends ChecksumCheckerBase like IPacketFilter
{
    parameters:
        checksumType = "ethernet-fcs";
        @class(EthernetFragmentFcsChecker);
        @display("i=block/checker");
}
