//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp;

//
// Represents a single TCP connection in the TCP protocol implementation.
// Implements the TCP state machine and handles connection establishment,
// data transfer, flow control, congestion control, and connection termination.
// Provides extensive statistics collection for various TCP metrics including
// window sizes, sequence numbers, RTT measurements, and congestion parameters.
//
simple TcpConnection {
    parameters:
        @signal[state];    // FSM state
        @signal[sndWnd];    // Snd_wnd
        @signal[rcvWnd];    // Rcv_wnd
        @signal[rcvAdv];    // Current advertised window (=rcv_adv)
        @signal[sndNxt];    // Sent seqNo
        @signal[sndAck];    // Sent ackNo
        @signal[rcvSeq];    // Received seqNo
        @signal[rcvAck];    // Received ackNo (=snd_una)
        @signal[unacked];    // Number of bytes unacknowledged
        @signal[dupAcks];    // Current number of received dupAcks
        @signal[pipe];    // Current sender's estimate of bytes outstanding in the network
        @signal[sndSacks];    // Number of sent Sacks
        @signal[rcvSacks];    // Number of received Sacks
        @signal[rcvOooSeg];    // Number of received out-of-order segments
        @signal[rcvNASeg];    // Number of received not acceptable segments
        @signal[sackedBytes];    // Current number of received sacked bytes
        @signal[tcpRcvQueueBytes];    // Current amount of used bytes in TCP receive queue
        @signal[tcpRcvQueueDrops];    // Number of drops in TCP receive queue
        @signal[tcpRcvPayloadBytes];   // Amount of payload bytes received (including duplicates, out of order, etc.) for TCP throughput

        //TcpAlgorithm signals:
        @signal[cwnd];    // Will record changes to snd_cwnd
        @signal[ssthresh];    // Will record changes to ssthresh
        @signal[rtt];    // Will record measured RTT
        @signal[srtt];    // Will record smoothed RTT
        @signal[rttvar];    // Will record RTT variance (rttvar)
        @signal[rto];    // Will record retransmission timeout
        @signal[numRtos];    // Will record total number of RTOs

        @signal[load];   // Will record load (currently only with DcTcp)
        @signal[calcLoad];    // Will record calcLoad (currently only with DcTcp)
        @signal[markingProb];   // Will record marking probability (currently only with DcTcp)

        @statistic[state](record=vector; interpolationmode=sample-hold);
        @statistic[sndWnd](record=vector; interpolationmode=sample-hold);    // Snd_wnd
        @statistic[rcvWnd](record=vector; interpolationmode=sample-hold);    // Rcv_wnd
        @statistic[rcvAdv](record=vector; interpolationmode=sample-hold);    // Current advertised window (=rcv_adv)
        @statistic[sndNxt](record=vector; interpolationmode=sample-hold);    // Sent seqNo
        @statistic[sndAck](record=vector; interpolationmode=sample-hold);    // Sent ackNo
        @statistic[rcvSeq](record=vector; interpolationmode=sample-hold);    // Received seqNo
        @statistic[rcvAck](record=vector; interpolationmode=sample-hold);    // Received ackNo (=snd_una)
        @statistic[unacked](record=vector; interpolationmode=sample-hold);    // Number of bytes unacknowledged
        @statistic[dupAcks](record=vector; interpolationmode=sample-hold);    // Current number of received dupAcks
        @statistic[pipe](record=vector; interpolationmode=sample-hold);    // Current sender's estimate of bytes outstanding in the network
        @statistic[sndSacks](record=vector; interpolationmode=sample-hold);    // Number of sent Sacks
        @statistic[rcvSacks](record=vector; interpolationmode=sample-hold);    // Number of received Sacks
        @statistic[rcvOooSeg](record=vector; interpolationmode=sample-hold);    // Number of received out-of-order segments
        @statistic[rcvNASeg](record=vector; interpolationmode=sample-hold);    // Number of received not acceptable segments
        @statistic[sackedBytes](record=vector; interpolationmode=sample-hold);    // Current number of received sacked bytes
        @statistic[tcpRcvQueueBytes](record=vector; interpolationmode=sample-hold);    // Current amount of used bytes in tcp receive queue
        @statistic[tcpRcvQueueDrops](record=vector; interpolationmode=sample-hold);    // Number of drops in tcp receive queue
        @statistic[tcpRcvPayloadBytes](record=vector; interpolationmode=sample-hold);    // Current amount of payload bytes received (including duplicates, out of order etc) for TCP throughput

        @statistic[cwnd](record=vector; interpolationmode=sample-hold);    // Will record changes to snd_cwnd
        @statistic[ssthresh](record=vector; interpolationmode=sample-hold);    // Will record changes to ssthresh
        @statistic[rtt](record=vector; interpolationmode=sample-hold);    // Will record measured RTT
        @statistic[srtt](record=vector; interpolationmode=sample-hold);    // Will record smoothed RTT
        @statistic[rttvar](record=vector; interpolationmode=sample-hold);    // Will record RTT variance (rttvar)
        @statistic[rto](record=vector; interpolationmode=sample-hold);    // Will record retransmission timeout
        @statistic[numRtos](record=vector; interpolationmode=sample-hold);    // Will record total number of RTOs

        @statistic[load](record=vector; interpolationmode=sample-hold);    // Will record load (currently only with DcTcp)
        @statistic[calcLoad](record=vector; interpolationmode=sample-hold);    // Will record calcLoad (currently only with DcTcp)
        @statistic[markingProb](record=vector; interpolationmode=sample-hold);    // Will record marking probability (currently only with DcTcp)
}

