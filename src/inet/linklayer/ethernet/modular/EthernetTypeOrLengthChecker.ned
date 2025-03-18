//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Examines the Type/Length field in Ethernet frames to determine whether it
// represents a type (Ethernet II) or a length (IEEE 802.3). For Ethernet II
// frames, it identifies the encapsulated protocol based on the EtherType value.
// For IEEE 802.3 frames, it sets the protocol to IEEE 802.2 LLC. The module
// adds appropriate protocol tags to packets for further processing and filters
// out packets with unknown EtherType values.
//
simple EthernetTypeOrLengthChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetTypeOrLengthChecker);
        @display("i=block/checker");
}
