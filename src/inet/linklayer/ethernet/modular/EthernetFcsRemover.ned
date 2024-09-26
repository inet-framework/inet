//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

simple EthernetFcsRemover extends PacketFilterBase like IPacketFilter
{
    parameters:
        bool popFcs = true;
        @class(EthernetFcsChecker);
        @display("i=block/checker");
    gates:
        in @interface[inet::queueing::IPassivePacketSink](pdu=ethernetmac) @interface[inet::queueing::IActivePacketSink](pdu=ethernetmac);
}
