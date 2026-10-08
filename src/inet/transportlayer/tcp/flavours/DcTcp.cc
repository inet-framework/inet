//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/DcTcp.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

namespace inet {
namespace tcp {

Register_Class(DcTcp);

simsignal_t DcTcp::loadSignal = cComponent::registerSignal("load"); // will record load
simsignal_t DcTcp::calcLoadSignal = cComponent::registerSignal("calcLoad"); // will record total number of RTOs
simsignal_t DcTcp::markingProbSignal = cComponent::registerSignal("markingProb"); // will record marking probability

DcTcp::DcTcp() : TcpReno(),
    state((DcTcpStateVariables *&)TcpAlgorithm::state)
{
}

void DcTcp::initialize()
{
    TcpReno::initialize();
    state->dctcp_gamma = conn->getTcpMain()->par("dctcpGamma");
}

bool DcTcp::processEce(uint32_t numBytesAcked)
{
    // DCTCP replaces the halving of RFC 3168 with a reduction in proportion to the
    // fraction of marked bytes (RFC 8257 section 3.3). A return value of true tells
    // the shared ACK path that cwnd changed, so this ACK does not also grow it.
    if (!state || !state->ect)
        return false;

    // RFC 8257 3.3.2
    state->dctcp_bytesAcked += numBytesAcked;

    // RFC 8257 3.3.3
    if (state->gotEce) {
        state->dctcp_bytesMarked += numBytesAcked;
        conn->emit(markingProbSignal, 1);
    }
    else {
        conn->emit(markingProbSignal, 0);
    }

    // RFC 8257 3.3.4
    if (state->snd_una > state->dctcp_windEnd) {
        // RFC 8257 3.3.5
        double ratio;

        ratio = ((double)state->dctcp_bytesMarked / state->dctcp_bytesAcked);
        conn->emit(loadSignal, ratio);

        // RFC 8257 3.3.6
        // DCTCP.Alpha = DCTCP.Alpha * (1 - g) + g * M
        state->dctcp_alpha = state->dctcp_alpha * (1 - state->dctcp_gamma) + state->dctcp_gamma * ratio;
        conn->emit(calcLoadSignal, state->dctcp_alpha);

        // RFC 8257 3.3.7
        state->dctcp_windEnd = state->snd_nxt;

        // RFC 8257 3.3.8
        state->dctcp_bytesAcked = state->dctcp_bytesMarked = 0;
        state->sndCwr = false;
    }

    // Applying DcTcp style cwnd update only if there was congestion and the window has not yet been reduced during current interval
    if (state->dctcp_bytesMarked && !state->sndCwr) {
        state->sndCwr = true;

        // RFC 8257 3.3.9
        state->snd_cwnd = state->snd_cwnd * (1 - state->dctcp_alpha / 2);

        conn->emit(cwndSignal, state->snd_cwnd);

        uint32_t flight_size = std::min(state->snd_cwnd, state->snd_wnd); // FIXME - Does this formula computes the amount of outstanding data?
        state->ssthresh = std::max(3 * flight_size / 4, 2 * state->snd_mss);

        conn->emit(ssthreshSignal, state->ssthresh);
        return true;
    }

    return false;
}

bool DcTcp::shouldMarkAck()
{
    // RFC 8257 3.2 page 6
    // When sending an ACK, the ECE flag MUST be set if and only if DCTCP.CE is true.
    return state->dctcp_ce;
}

void DcTcp::processEcnInEstablished()
{
    if (state && state->ect) {
        // RFC 8257 3.2.1
        if (state->gotCeIndication && !state->dctcp_ce) {
            state->dctcp_ce = true;
            state->ack_now = true;
        }

        // RFC 8257 3.2.2
        if (!state->gotCeIndication && state->dctcp_ce) {
            state->dctcp_ce = false;
            state->ack_now = true;
        }
    }
}

} // namespace tcp
} // namespace inet

