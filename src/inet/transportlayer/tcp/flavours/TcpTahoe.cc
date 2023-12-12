//
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpTahoe.h"

namespace inet {
namespace tcp {

Register_Class(TcpTahoe);

TcpTahoe::TcpTahoe() : TcpAlgorithmBase(),
    state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
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
    if (state->dupacks != 0) {
        state->dupacks = 0;
        conn->emit(dupAcksSignal, state->dupacks);
    }
    sendData(false);
}

void TcpTahoe::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    bool isDupack = state->snd_una == tcpHeader->getAckNo() && payloadLength == 0 && state->snd_una != state->snd_max;
    if (isDupack) {
        state->dupacks++;
        conn->emit(dupAcksSignal, state->dupacks);

        if (state->dupacks == state->dupthresh)
            resetToSlowStart();
        else
            sendData(false);
    }
    else
        sendData(false);
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

