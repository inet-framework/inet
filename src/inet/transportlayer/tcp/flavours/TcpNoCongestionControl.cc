//
// Copyright (C) 2004 Andras Varga
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

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/flavours/TcpNoCongestionControl.h"

namespace inet {
namespace tcp {

Register_Class(TcpNoCongestionControl);

TcpNoCongestionControl::TcpNoCongestionControl() : TcpBaseAlg(),
    state((TcpNoCongestionControlStateVariables *&)TcpAlgorithm::state)
{
}

void TcpNoCongestionControl::initialize()
{
    TcpBaseAlg::initialize();

    // set congestion window to a practically infinite value
    state->snd_cwnd = 0x7fffffff;
}

void TcpNoCongestionControl::established(bool active)
{
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData(false)) // FIXME TODO - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
            conn->sendAck();
    }
}

bool TcpNoCongestionControl::sendData(bool sendCommandInvoked)
{
    //
    // Nagle's algorithm: when a TCP connection has outstanding data that has not
    // yet been acknowledged, small segments cannot be sent until the outstanding
    // data is acknowledged. (In this case, small amounts of data are collected
    // by TCP and sent in a single segment.)
    //
    // FIXME there's also something like this: can still send if
    // "b) a segment that can be sent is at least half the size of
    // the largest window ever advertised by the receiver"

    bool fullSegmentsOnly = sendCommandInvoked && state->nagle_enabled && state->snd_una != state->snd_max;

    if (fullSegmentsOnly)
        EV_INFO << "Nagle is enabled and there's unacked data: only full segments will be sent\n";

    //
    // Send window is effectively the minimum of the congestion window (cwnd)
    // and the advertised window (snd_wnd).
    //
    return conn->sendData(fullSegmentsOnly, state->snd_cwnd);
}

void TcpNoCongestionControl::processRexmitTimer(TcpEventCode& event)
{
    TcpBaseAlg::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // Tahoe-style retransmission: only one segment
    conn->retransmitOneSegment(true);

    ASSERT(state->snd_cwnd == 0x7fffffff);
}

void TcpNoCongestionControl::receivedDataAck(uint32 firstSeqAcked)
{
    TcpBaseAlg::receivedDataAck(firstSeqAcked);

    // ack may have freed up some room in the window, try sending
    sendData(false);

    ASSERT(state->snd_cwnd == 0x7fffffff);
}

} // namespace tcp
} // namespace inet

