//
// Copyright (C) 2004-2005 Andras Varga
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

#include <algorithm>    // min,max
#include "inet/transportlayer/tcp/flavours/TCPTahoe.h"
#include "inet/transportlayer/tcp/TCP.h"

namespace inet {

namespace tcp {

Register_Class(TCPTahoe);

TCPTahoe::TCPTahoe() : TCPTahoeRenoFamily(),
    state((TCPTahoeStateVariables *&)TCPAlgorithm::state)
{
}

void TCPTahoe::recalculateSlowStartThreshold()
{
    // set ssthresh to flight size / 2, but at least 2 MSS
    // (the formula below practically amounts to ssthresh = cwnd / 2 most of the time)
    uint32 flight_size = std::min(state->snd_cwnd, state->snd_wnd);    // FIXME TODO - Does this formula computes the amount of outstanding data?
    // uint32 flight_size = state->snd_max - state->snd_una;
    state->ssthresh = std::max(flight_size / 2, 2 * state->snd_mss);

    if (ssthreshVector)
        ssthreshVector->record(state->ssthresh);
}

void TCPTahoe::processRexmitTimer(TCPEventCode& event)
{
    TCPTahoeRenoFamily::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // begin Slow Start (RFC 2581)
    recalculateSlowStartThreshold();
    state->snd_cwnd = state->snd_mss;

    if (cwndVector)
        cwndVector->record(state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;

    // Tahoe retransmits only one segment at the front of the queue
    conn->retransmitOneSegment(true);
}

void TCPTahoe::receivedDataAck(uint32 firstSeqAcked)
{
    TCPTahoeRenoFamily::receivedDataAck(firstSeqAcked);

    //
    // Perform slow start and congestion avoidance.
    //
    if (state->snd_cwnd < state->ssthresh) {
        EV_DETAIL << "cwnd <= ssthresh: Slow Start: increasing cwnd by SMSS bytes to ";

        // perform Slow Start. RFC 2581: "During slow start, a TCP increments cwnd
        // by at most SMSS bytes for each ACK received that acknowledges new data."
        state->snd_cwnd += state->snd_mss;

        // Note: we could increase cwnd based on the number of bytes being
        // acknowledged by each arriving ACK, rather than by the number of ACKs
        // that arrive. This is called "Appropriate Byte Counting" (ABC) and is
        // described in RFC 3465 (experimental).
        //
        // int bytesAcked = state->snd_una - firstSeqAcked;
        // state->snd_cwnd += bytesAcked;

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);

        EV_DETAIL << "cwnd=" << state->snd_cwnd << "\n";
    }
    else {
        // perform Congestion Avoidance (RFC 2581)
        int incr = state->snd_mss * state->snd_mss / state->snd_cwnd;

        if (incr == 0)
            incr = 1;

        state->snd_cwnd += incr;

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);

        //
        // Note: some implementations use extra additive constant mss / 8 here
        // which is known to be incorrect (RFC 2581 p5)
        //
        // Note 2: RFC 3465 (experimental) "Appropriate Byte Counting" (ABC)
        // would require maintaining a bytes_acked variable here which we don't do
        //

        EV_DETAIL << "cwnd>ssthresh: Congestion Avoidance: increasing cwnd linearly, to " << state->snd_cwnd << "\n";
    }

    // ack and/or cwnd increase may have freed up some room in the window, try sending
    sendData(false);
}

void TCPTahoe::receivedDuplicateAck()
{
    TCPTahoeRenoFamily::receivedDuplicateAck();

    if (state->dupacks == DUPTHRESH) {    // DUPTHRESH = 3
        EV_DETAIL << "Tahoe on dupAcks == DUPTHRESH(=3): perform Fast Retransmit, and enter Slow Start:\n";

        // enter Slow Start
        recalculateSlowStartThreshold();
        state->snd_cwnd = state->snd_mss;

        if (cwndVector)
            cwndVector->record(state->snd_cwnd);

        EV_DETAIL << "Set cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";

        // Fast Retransmission: retransmit missing segment without waiting
        // for the REXMIT timer to expire
        conn->retransmitOneSegment(false);

        // Do not restart REXMIT timer.
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        // Resetting the REXMIT timer is discussed in RFC 2582/3782 (NewReno) and RFC 2988.
    }
}

} // namespace tcp

} // namespace inet

