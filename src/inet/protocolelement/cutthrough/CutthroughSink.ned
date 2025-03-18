//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketStreamer;

//
// TODO
//
simple CutthroughSink extends PacketStreamer
{
    parameters:
        @class(CutthroughSink);
        @display("i=block/routing");
}
