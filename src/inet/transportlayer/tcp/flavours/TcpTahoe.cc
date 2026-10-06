//
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpTahoe.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

namespace inet {
namespace tcp {

Register_Class(TcpTahoe);

TcpTahoe::TcpTahoe() : TcpAlgorithmBase(),
    state((TcpTahoeStateVariables *&)TcpAlgorithm::state)
{
}

void TcpTahoe::initialize()
{
    TcpAlgorithmBase::initialize();

    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

void TcpTahoe::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    resetToSlowStart();
}

void TcpTahoe::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);

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
    TcpAlgorithmBase::receivedDuplicateAck();

    // Tahoe has no fast recovery: the third duplicate ACK starts the same
    // slow start from the first unacknowledged segment as a timeout does
    if (state->dupacks == state->dupthresh) {
        EV_DETAIL << "Tahoe on dupAcks == DUPTHRESH(=" << state->dupthresh << ": perform Fast Retransmit, and enter Slow Start:\n";
        resetToSlowStart();
    }
}

void TcpTahoe::resetToSlowStart()
{
    state->ssthresh = std::max(state->snd_cwnd / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);
    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);
    EV_INFO << "Beginning slow start" << EV_FIELD(ssthresh, state->ssthresh) << EV_FIELD(cwnd, state->snd_cwnd) << EV_ENDL;
    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

} // namespace tcp
} // namespace inet

