//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;

//
// This module implements Random Early Detection (RED).
//
// It has n input and n output gates (specified by the 'numGates'
// parameter). Packets arrived at the ith input gate are
// forwarded to the ith output gate, or dropped.
//
// The module sums the used buffer space of the queues attached
// to the output gates. If it is below a minimum threshold,
// the packet won't be dropped, if above a maximum threshold,
// it will be dropped, if it is between the minimum and
// maximum threshold, it will be dropped by a given probability.
// This probability determined by a linear function which is
// 0 at the minth and maxp at maxth.
//
// The queue length can be smoothed by specifying the 'wq'
// parameter. The average queue length used in the tests
// are computed by the formula:
//
//  avg = (1-wq)*avg + wq*qlen
//
// The minth, maxth, and maxp parameters can be specified
// separately for each input gate, so this module can be
// used to implement different packet drop priorities.
//
simple RedDropper extends PacketFilterBase
{
    parameters:
        string collectionModule = default("");
        double wq = default(0.002);  // weight of the current queue length in the averaged queue length, in range [0.0, 1.0]
        double minth = default(5);  // minimum threshold for avg queue length
        double maxth = default(50);  // maximum threshold for avg queue length (=buffer capacity), in range (minth,packetCapacity]
        double maxp = default(0.02);  // maximum value for pbs, in range [0.0, 1.0]
        double pkrate = default(150);  // average packet rate for calculations when queue is empty
        bool useEcn = default(false); // if enabled, packets are marked with ECN if applicable
        int packetCapacity = default(int(maxth)); // packets are dropped if queue length is greater
        @class(RedDropper);
        @display("i=block/downarrow");
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropCongestion](title="packet drop: congestion"; source=packetDropReasonIsCongestion(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

