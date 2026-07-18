//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.lifetime;

import inet.queueing.base.PacketFlowBase;

//
// Stamps each packet with a lifetime header recording its creation time, so a downstream
// ~ReceiveWithLifetime can drop packets that have been in flight too long. The time-based
// analogue of ~SendWithHopLimit.
//
simple SendWithLifetime extends PacketFlowBase
{
    parameters:
        @class(SendWithLifetime);
        @display("i=block/tx");
}
