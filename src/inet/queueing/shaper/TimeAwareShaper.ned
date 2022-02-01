//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

//
// This module implements a time-aware shaper using a gate submodule that opens
// and closes according to a time-based schedule.
//
module TimeAwareShaper extends PacketShaper
{
    parameters:
        gate.typename = default("PeriodicGate");
}
