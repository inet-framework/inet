//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.lifetime;

import inet.queueing.base.PacketFilterBase;

//
// Drops packets whose age (now minus the creation time stamped by ~SendWithLifetime) exceeds
// ~maxLifetime; otherwise pops the lifetime header and forwards. The time-based analogue of
// ~ReceiveWithHopLimit.
//
simple ReceiveWithLifetime extends PacketFilterBase
{
    parameters:
        @class(ReceiveWithLifetime);
        double maxLifetime @unit(s); // packets older than this when they arrive here are dropped
        @display("i=block/rx");
}
