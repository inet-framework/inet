//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This packet gate module allows or forbids packets to pass through depending
// on whether the gate is open or closed as specified by the parameter. The gate
// can also be controlled by the user by manually changing the open parameter
// from the graphical user interface while the simulation is paused.
//
simple InteractiveGate extends PacketGateBase like IPacketGate
{
    parameters:
        bool open @mutable = default(true);
        @class(InteractiveGate);
}
