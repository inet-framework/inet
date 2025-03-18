//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Inserts the Type/Length field in Ethernet frames. Determines whether to use a
// type value (Ethernet II) or a length value (IEEE 802.3) based on the packet's
// encapsulated protocol. For Ethernet II frames, sets the appropriate EtherType
// value corresponding to the encapsulated protocol (e.g., IPv4, IPv6, ARP). For
// IEEE 802.3 frames, calculates and sets the length of the data field. Works as
// a counterpart to the EthernetTypeOrLengthChecker module.
//
simple EthernetTypeOrLengthInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetTypeOrLengthInserter);
        @display("i=block/inserter");
}
