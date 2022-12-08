//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketStreamer;

simple EthernetCutthroughSink extends PacketStreamer
{
    parameters:
        @class(EthernetCutthroughSink);
        @display("i=block/routing");
}
