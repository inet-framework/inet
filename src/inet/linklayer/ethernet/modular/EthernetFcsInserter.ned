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
// Adds Frame Check Sequence (FCS) to Ethernet frames. Inserts the FCS trailer
// at the end of outgoing packets with configurable mode (disabled, declared
// correct, or computed). For cutthrough packets, retrieves the trailer chunk
// from the cutthrough tag. Sets the protocol tag to indicate Ethernet MAC
// protocol for further processing.
//
simple EthernetFcsInserter extends ChecksumInserterBase like IPacketFlow
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        checksumMode = fcsMode;
        checksumType = "ethernet-fcs";
        @class(EthernetFcsInserter);
        @display("i=block/inserter");
}
