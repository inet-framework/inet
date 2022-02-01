//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

simple EthernetAddressChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        bool promiscuous = default(false);
        @class(EthernetAddressChecker);
        @display("i=block/checker");
}
