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

package inet.transportlayer.tcp;

simple TcpConnection {
    parameters:
        @signal[state];    // FSM state
        @signal[sndWnd];    // snd_wnd
        @signal[rcvWnd];    // rcv_wnd
        @signal[rcvAdv];    // current advertised window (=rcv_adv)
        @signal[sndNxt];    // sent seqNo
        @signal[sndAck];    // sent ackNo
        @signal[rcvSeq];    // received seqNo
        @signal[rcvAck];    // received ackNo (=snd_una)
        @signal[unacked];    // number of bytes unacknowledged
        @signal[dupAcks];    // current number of received dupAcks
        @signal[pipe];    // current sender's estimate of bytes outstanding in the network
        @signal[sndSacks];    // number of sent Sacks
        @signal[rcvSacks];    // number of received Sacks
        @signal[rcvOooSeg];    // number of received out-of-order segments
        @signal[rcvNASeg];    // number of received not acceptable segments
        @signal[sackedBytes];    // current number of received sacked bytes
        @signal[tcpRcvQueueBytes];    // current amount of used bytes in tcp receive queue
        @signal[tcpRcvQueueDrops];    // number of drops in tcp receive queue

        //TcpAlgorithm signals:
        @signal[cwnd];    // will record changes to snd_cwnd
        @signal[ssthresh];    // will record changes to ssthresh
        @signal[rtt];    // will record measured RTT
        @signal[srtt];    // will record smoothed RTT
        @signal[rttvar];    // will record RTT variance (rttvar)
        @signal[rto];    // will record retransmission timeout
        @signal[numRtos];    // will record total number of RTOs

        @statistic[state](record=vector; interpolationmode=sample-hold);
        @statistic[sndWnd](record=vector; interpolationmode=sample-hold);    // snd_wnd
        @statistic[rcvWnd](record=vector; interpolationmode=sample-hold);    // rcv_wnd
        @statistic[rcvAdv](record=vector; interpolationmode=sample-hold);    // current advertised window (=rcv_adv)
        @statistic[sndNxt](record=vector; interpolationmode=sample-hold);    // sent seqNo
        @statistic[sndAck](record=vector; interpolationmode=sample-hold);    // sent ackNo
        @statistic[rcvSeq](record=vector; interpolationmode=sample-hold);    // received seqNo
        @statistic[rcvAck](record=vector; interpolationmode=sample-hold);    // received ackNo (=snd_una)
        @statistic[unacked](record=vector; interpolationmode=sample-hold);    // number of bytes unacknowledged
        @statistic[dupAcks](record=vector; interpolationmode=sample-hold);    // current number of received dupAcks
        @statistic[pipe](record=vector; interpolationmode=sample-hold);    // current sender's estimate of bytes outstanding in the network
        @statistic[sndSacks](record=vector; interpolationmode=sample-hold);    // number of sent Sacks
        @statistic[rcvSacks](record=vector; interpolationmode=sample-hold);    // number of received Sacks
        @statistic[rcvOooSeg](record=vector; interpolationmode=sample-hold);    // number of received out-of-order segments
        @statistic[rcvNASeg](record=vector; interpolationmode=sample-hold);    // number of received not acceptable segments
        @statistic[sackedBytes](record=vector; interpolationmode=sample-hold);    // current number of received sacked bytes
        @statistic[tcpRcvQueueBytes](record=vector; interpolationmode=sample-hold);    // current amount of used bytes in tcp receive queue
        @statistic[tcpRcvQueueDrops](record=vector; interpolationmode=sample-hold);    // number of drops in tcp receive queue

        @statistic[cwnd](record=vector; interpolationmode=sample-hold);    // will record changes to snd_cwnd
        @statistic[ssthresh](record=vector; interpolationmode=sample-hold);    // will record changes to ssthresh
        @statistic[rtt](record=vector; interpolationmode=sample-hold);    // will record measured RTT
        @statistic[srtt](record=vector; interpolationmode=sample-hold);    // will record smoothed RTT
        @statistic[rttvar](record=vector; interpolationmode=sample-hold);    // will record RTT variance (rttvar)
        @statistic[rto](record=vector; interpolationmode=sample-hold);    // will record retransmission timeout
        @statistic[numRtos](record=vector; interpolationmode=sample-hold);    // will record total number of RTOs
}

