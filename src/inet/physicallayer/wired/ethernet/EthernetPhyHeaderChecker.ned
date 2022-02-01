//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

simple EthernetPhyHeaderChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetPhyHeaderChecker);
        @display("i=block/checker");
}
