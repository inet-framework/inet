//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module allows or forbids packets to pass through depending on whether
// the gate is open or closed.
//
simple PacketGate extends PacketGateBase like IPacketGate
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        bool initiallyOpen = default(false);
        double openTime @unit(s);
        double closeTime @unit(s);
        @class(PacketGate);
}
