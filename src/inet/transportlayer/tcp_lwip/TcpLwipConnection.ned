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
        @signal[sndWnd];    // snd_wnd
        @signal[sndNxt];    // sent seqNo
        @signal[sndAck];    // sent ackNo
        @signal[rcvWnd];    // rcv_wnd
        @signal[rcvSeq];    // received seqNo
        @signal[rcvAck];    // received ackNo (=snd_una)

        @statistic[sndWnd](record=vector; interpolationmode=sample-hold);    // snd_wnd
        @statistic[sndNxt](record=vector; interpolationmode=sample-hold);    // sent seqNo
        @statistic[sndAck](record=vector; interpolationmode=sample-hold);    // sent ackNo
        @statistic[rcvWnd](record=vector; interpolationmode=sample-hold);    // rcv_wnd
        @statistic[rcvSeq](record=vector; interpolationmode=sample-hold);    // received seqNo
        @statistic[rcvAck](record=vector; interpolationmode=sample-hold);    // received ackNo (=snd_una)
}

