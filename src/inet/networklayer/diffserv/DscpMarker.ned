//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.diffserv;

import inet.queueing.base.PacketProcessorBase;

//
// This module sets the DSCP field (lower six bit of Tos/TrafficClass) of IP datagrams
// to the value specified by the dscps parameter.
//
// The dscps parameter is a space separated list of DSCP values.
// Packets arrived at gate in[i] are marked with the ith value. If there are fewer
// dscp values given than input gates, then the last one is repeated.
//
simple DscpMarker extends PacketProcessorBase
{
    parameters:
        string dscps; // space separated list if dscp values; both names (e.g. AF11, EF) and numbers (0x0A,0b101110) can be used
        @class(DscpMarker);
        @display("i=block/star");
        @signal[packetMarked](type=cPacket);
        @statistic[packetMarked](title="packets marked"; source=packetMarked; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input in[];
        output out;
}

