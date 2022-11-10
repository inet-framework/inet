//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketFlowBase;

//
// This module prevents backpressure propagation from its output to its input
// and vice verse. It always allows pushing and pulling packets independently
// of what the connected modules would allow to do in their current state. Both
// pushed and pulled packets pass through unchanged, but the operations may raise
// a runtime error due to this module preventing backpressure propagation.
//
simple BackPressureBarrier extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        @class(BackPressureBarrier);
        @display("i=block/segm");
}
