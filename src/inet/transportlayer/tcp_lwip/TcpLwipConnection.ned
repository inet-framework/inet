//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.tcp_lwip;

simple TcpLwipConnection {
    parameters:
        @signal[state];    // FSM state
        @signal[sndWnd];    // Snd_wnd
        @signal[sndNxt];    // Sent seqNo
        @signal[sndAck];    // Sent ackNo
        @signal[rcvWnd];    // Rcv_wnd
        @signal[rcvSeq];    // Received seqNo
        @signal[rcvAck];    // Received ackNo (=snd_una)

        @statistic[sndWnd](record=vector; interpolationmode=sample-hold);    // Snd_wnd
        @statistic[sndNxt](record=vector; interpolationmode=sample-hold);    // Sent seqNo
        @statistic[sndAck](record=vector; interpolationmode=sample-hold);    // Sent ackNo
        @statistic[rcvWnd](record=vector; interpolationmode=sample-hold);    // Rcv_wnd
        @statistic[rcvSeq](record=vector; interpolationmode=sample-hold);    // Received seqNo
        @statistic[rcvAck](record=vector; interpolationmode=sample-hold);    // Received ackNo (=snd_una)
}

