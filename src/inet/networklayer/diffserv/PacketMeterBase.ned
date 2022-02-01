//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

import inet.queueing.base.PacketProcessorBase;


simple PacketMeterBase extends PacketProcessorBase
{
    parameters:
        @display("i=block/classifier");
        @signal[packetPushed](type=inet::Packet);
        @signal[packetPulled](type=inet::Packet);
        @statistic[packetPushed](title="packets pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetPulled](title="packets pulled"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input in;
}
