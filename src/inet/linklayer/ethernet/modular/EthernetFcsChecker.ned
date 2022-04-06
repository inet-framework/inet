//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

simple EthernetFcsChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        bool popFcs = default(true);
        @class(EthernetFcsChecker);
        @display("i=block/checker");
}
