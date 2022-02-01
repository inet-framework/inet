//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFilterBase;

simple Ieee8022LlcChecker extends PacketFilterBase like IIeee8022LlcChecker
{
    parameters:
        @class(Ieee8022LlcChecker);
        @display("i=block/checker");
        @signal[packetDropped](type=inet::Packet);
}

