//
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpTahoeWithoutFastRetransmit.h"

namespace inet {
namespace tcp {

Register_Class(TcpTahoeWithoutFastRetransmit);

TcpTahoeWithoutFastRetransmit::TcpTahoeWithoutFastRetransmit() : TcpAlgorithmBase(),
    state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
{
}

void TcpTahoeWithoutFastRetransmit::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

void TcpTahoeWithoutFastRetransmit::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    state->ssthresh = std::max(state->snd_cwnd / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Beginning slow start" << EV_FIELD(ssthresh, state->ssthresh) << EV_FIELD(cwnd, state->snd_cwnd) << EV_ENDL;

    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

void TcpTahoeWithoutFastRetransmit::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);
    if (state->snd_cwnd < state->ssthresh) {
        state->snd_cwnd += state->snd_mss;
        conn->emit(cwndSignal, state->snd_cwnd);
        EV_INFO << "Incrementing cwnd in slow start" << EV_FIELD(cwnd, state->snd_cwnd) << EV_ENDL;
    }
    else {
        // congestion avoidance
        state->snd_cwnd += std::max(1u, state->snd_mss * state->snd_mss / state->snd_cwnd);
        conn->emit(cwndSignal, state->snd_cwnd);
        EV_INFO << "Incrementing cwnd in congestion avoidance" << EV_FIELD(cwnd, state->snd_cwnd) << EV_ENDL;
    }
    sendData(false);
}

} // namespace tcp
} // namespace inet

