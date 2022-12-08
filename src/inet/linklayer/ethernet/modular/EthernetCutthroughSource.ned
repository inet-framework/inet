//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketDestreamer;

simple EthernetCutthroughSource extends PacketDestreamer
{
    parameters:
        string macTableModule;
        int cutthroughSwitchingHeaderSize @unit(b) = default(0b);
        @class(EthernetCutthroughSource);
        @display("i=block/routing");
}
