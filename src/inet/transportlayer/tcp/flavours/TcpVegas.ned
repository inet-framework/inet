//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp.flavours;

//
// Vegas: the window follows the difference of the expected and the actual
// rate.
//
simple TcpVegas extends TcpAlgorithm
{
    parameters:
        @class(TcpVegas);
        // the thresholds of Vegas, in segments: the difference of the expected and the actual rate, times the base RTT, is the estimate of the extra segments in the network
        int alpha = default(2); // in congestion avoidance, below this many extra segments, cwnd grows by one segment for each RTT
        int beta = default(4); // in congestion avoidance, above this many extra segments, cwnd shrinks by one segment for each RTT
        int gamma = default(1); // in slow start, above this many extra segments, Vegas leaves slow start
}
