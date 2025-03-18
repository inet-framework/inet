//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Verifies the Frame Check Sequence (FCS) in Ethernet frame fragments to detect
// transmission errors. Filters out fragments with incorrect FCS and maintains
// state between fragments to verify the complete frame's FCS. Updates fragment
// tags to indicate if a fragment is the last one based on the mCRC field and
// removes the FCS from fragments that pass the check.
//
simple EthernetFragmentFcsChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetFragmentFcsChecker);
        @display("i=block/checker");
}
