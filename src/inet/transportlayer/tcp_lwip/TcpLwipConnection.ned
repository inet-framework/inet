//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2010 Thomas Reschka
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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

