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
// Verifies the Frame Check Sequence (FCS) in Ethernet frames to detect
// transmission errors. Filters out frames with incorrect FCS and optionally
// removes the FCS from frames that pass the check. Supports different FCS modes
// including declared correct, declared incorrect, and computed. For cutthrough
// packets, stores the trailer chunk in the cutthrough tag.
//
simple EthernetFcsChecker extends ChecksumCheckerBase like IPacketFilter
{
    parameters:
        checksumType = "ethernet-fcs";
        bool popFcs = default(true);
        @class(EthernetFcsChecker);
        @display("i=block/checker");
}
