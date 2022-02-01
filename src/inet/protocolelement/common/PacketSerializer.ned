//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;

//
// This module serializes packets flowing from its input to its output.
//
simple PacketSerializer extends PacketFlowBase
{
    parameters:
        @display("i=block/cogwheel");
        @class(PacketSerializer);
}
