//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketDestreamer;

simple CutthroughSource extends PacketDestreamer
{
    parameters:
        int cutthroughPosition @unit(b);
        @class(CutthroughSource);
        @display("i=block/routing");
}
