//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFilterBase;

simple ProtocolChecker extends PacketFilterBase
{
    parameters:
        @class(ProtocolChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}

