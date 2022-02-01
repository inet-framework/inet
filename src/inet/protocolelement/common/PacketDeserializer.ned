//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;

simple PacketDeserializer extends PacketFlowBase
{
    parameters:
        @display("i=block/cogwheel");
        @class(PacketDeserializer);
}
