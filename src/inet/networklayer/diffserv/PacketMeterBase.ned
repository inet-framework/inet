//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.networklayer.diffserv;

import inet.queueing.base.PacketProcessorBase;


//
// Base module for packet meters in the DiffServ architecture. Provides common
// functionality for different meter implementations that classify packets
// according to various traffic conditioning algorithms. Derived meters are used
// to determine whether packets conform to traffic profiles and mark them
// accordingly for differentiated quality of service.
//
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
