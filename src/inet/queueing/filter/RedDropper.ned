//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.filter;

import inet.queueing.base.PacketFilterBase;

//
// Active Queue Management (AQM) implementation of Random Early Detection (RED)
// algorithm. This module provides congestion avoidance by probabilistically
// dropping packets before queues overflow.
//
// Operation mechanism:
// - Monitors average queue length using an exponential weighted moving average
// - Drops packets based on current average queue length:
//   * Below minimum threshold (minth): no packets are dropped
//   * Above maximum threshold (maxth): all packets are dropped
//   * Between thresholds: packets dropped with linearly increasing probability
//     from 0 at minth to maxp at maxth
// - Optionally marks packets with Explicit Congestion Notification (ECN)
//   instead of dropping them when useEcn is enabled
//
// The average queue length is calculated using:
//   avg = (1-wq)*avg + wq*qlen
// where wq is the queue weight parameter that determines how quickly
// the average adapts to changes in the actual queue length.
//
// The module supports multiple gates, allowing different drop parameters
// for different traffic classes or priorities.
//
simple RedDropper extends PacketFilterBase
{
    parameters:
        string collectionModule = default("");  // Module path to the queue to monitor; if empty, the connected consumer module is used

        // RED algorithm parameters
        double wq = default(0.002);  // Queue weight for the exponential weighted moving average calculation, in range [0.0, 1.0].
                                     // Lower values make the average less sensitive to short-term queue length changes.

        double minth = default(5);   // Minimum threshold for average queue length. No packets are dropped when avg < minth.
                                     // This should be set high enough to maximize network utilization.

        double maxth = default(50);  // Maximum threshold for average queue length, in range (minth,packetCapacity].
                                     // All packets are dropped when avg > maxth. This should be set low enough to avoid excessive delays.

        double maxp = default(0.02); // Maximum packet dropping probability when avg is exactly at maxth, in range [0.0, 1.0].
                                     // Higher values provide more aggressive congestion control.

        double pkrate = default(150); // Average packet rate (packets/sec) used for calculations when the queue is empty.
                                      // This helps maintain the average queue size calculation during idle periods.

        bool useEcn = default(false); // If enabled, packets are marked with ECN instead of being dropped when applicable.
                                      // This requires ECN-capable endpoints to be effective.

        int packetCapacity = default(int(maxth)); // Hard limit on queue length. Packets are dropped if queue length exceeds this value,
                                                  // regardless of the RED algorithm's decision.

        @class(RedDropper);
        @display("i=block/downarrow");
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropCongestion](title="packet drop: congestion"; source=packetDropReasonIsCongestion(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

