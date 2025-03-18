//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.checksum.base.ChecksumInserterBase;
import inet.queueing.contract.IPacketFlow;

//
// Adds Frame Check Sequence (FCS) to Ethernet frame fragments. Inserts the FCS
// trailer at the end of outgoing fragments with configurable mode (disabled,
// declared correct, or computed). Maintains state between fragments to compute
// the complete frame's FCS. Sets the mCRC field to indicate if a fragment is
// the last one and computes different FCS values for intermediate fragments
// versus the last fragment.
//
simple EthernetFragmentFcsInserter extends ChecksumInserterBase like IPacketFlow
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        checksumMode = fcsMode;
        checksumType = "ethernet-fcs";
        @class(EthernetFragmentFcsInserter);
        @display("i=block/inserter");
}
