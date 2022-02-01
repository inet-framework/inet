//
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpTahoe.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

Register_Class(TcpTahoe);

TcpTahoe::TcpTahoe() : TcpTahoeRenoFamily(),
    state((TcpTahoeStateVariables *&)TcpAlgorithm::state)
{
}

void TcpTahoe::recalculateSlowStartThreshold()
{
    // set ssthresh to flight size / 2, but at least 2 MSS
    // (the formula below practically amounts to ssthresh = cwnd / 2 most of the time)
    uint32_t flight_size = std::min(state->snd_cwnd, state->snd_wnd); // FIXME - Does this formula computes the amount of outstanding data?
//    uint32_t flight_size = state->snd_max - state->snd_una;
    state->ssthresh = std::max(flight_size / 2, 2 * state->snd_mss);

    conn->emit(ssthreshSignal, state->ssthresh);
}

void TcpTahoe::processRexmitTimer(TcpEventCode& event)
{
    TcpTahoeRenoFamily::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // begin Slow Start (RFC 2581)
    recalculateSlowStartThreshold();
    state->snd_cwnd = state->snd_mss;

    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;

    // Tahoe retransmits only one segment at the front of the queue
    conn->retransmitOneSegment(true);
}

void TcpTahoe::receivedDataAck(uint32_t firstSeqAcked)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

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
//        int bytesAcked = state->snd_una - firstSeqAcked;
//        state->snd_cwnd += bytesAcked;

        conn->emit(cwndSignal, state->snd_cwnd);

        EV_DETAIL << "cwnd=" << state->snd_cwnd << "\n";
    }
    else {
        // perform Congestion Avoidance (RFC 2581)
        int incr = state->snd_mss * state->snd_mss / state->snd_cwnd;

        if (incr == 0)
            incr = 1;

        state->snd_cwnd += incr;

        conn->emit(cwndSignal, state->snd_cwnd);

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

void TcpTahoe::receivedDuplicateAck()
{
    TcpTahoeRenoFamily::receivedDuplicateAck();

    if (state->dupacks == state->dupthresh) {
        EV_DETAIL << "Tahoe on dupAcks == DUPTHRESH(=" << state->dupthresh << ": perform Fast Retransmit, and enter Slow Start:\n";

        // enter Slow Start
        recalculateSlowStartThreshold();
        state->snd_cwnd = state->snd_mss;

        conn->emit(cwndSignal, state->snd_cwnd);

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

