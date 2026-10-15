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

Define_Module(DcTcp);

simsignal_t DcTcp::loadSignal = cComponent::registerSignal("load"); // will record load
simsignal_t DcTcp::calcLoadSignal = cComponent::registerSignal("calcLoad"); // will record total number of RTOs
simsignal_t DcTcp::markingProbSignal = cComponent::registerSignal("markingProb"); // will record marking probability

DcTcp::DcTcp() : TcpReno(),
    state((DcTcpStateVariables *&)TcpAlgorithm::state)
{
}

void DcTcp::initializeAlgorithm()
{
    TcpReno::initializeAlgorithm();
    state->dctcp_gamma = conn->getTcpMain()->par("dctcpGamma");
}

bool DcTcp::processEce(uint32_t numBytesAcked)
{
    // DCTCP replaces the halving of RFC 3168 with a reduction in proportion to the
    // fraction of marked bytes (RFC 8257 section 3.3). A return value of true tells
    // the shared ACK path that cwnd changed, so this ACK does not also grow it.
    if (!state || !state->ect)
        return false;

    countBytes(numBytesAcked);

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

void DcTcp::ackedInFastRecovery(uint32_t numBytesAcked)
{
    // RFC 8257 section 3.3 counts the acknowledged and the marked bytes at each ACK,
    // and Linux updates alpha at each ACK (dctcp_update_alpha()); only the cut of
    // cwnd waits until no recovery runs
    if (state->ect) {
        EV_INFO << "DCTCP: counting the " << numBytesAcked << " acknowledged bytes in a fast recovery\n";
        countBytes(numBytesAcked);
    }
}

void DcTcp::countBytes(uint32_t numBytesAcked)
{
    // RFC 8257 3.3.2
    state->dctcp_bytesAcked += numBytesAcked;

    // RFC 8257 3.3.3. With AccECN (RFC 9768), the AccECN option gives the marked bytes;
    // before an option with the ECEB field arrives, the new CE marks of this ACK times
    // the MSS do. With classic ECN, each byte that an ACK with ECE acknowledges counts
    // as marked.
    if (state->accEcnNegotiated) {
        uint32_t cePkts = state->deliveredCePkts - state->dctcp_deliveredCePktsMark;
        uint32_t ceBytes = state->deliveredCeBytes - state->dctcp_deliveredCeBytesMark;
        state->dctcp_deliveredCePktsMark = state->deliveredCePkts;
        state->dctcp_deliveredCeBytesMark = state->deliveredCeBytes;
        state->gotEce = false;
        uint32_t marked = state->accEcnCebSeen ? ceBytes : cePkts * state->snd_mss;
        state->dctcp_bytesMarked += marked;
        if (marked > 0)
            EV_INFO << "DCTCP: " << marked << " bytes newly marked (AccECN " << (state->accEcnCebSeen ? "option" : "ACE field")
                    << "), " << state->dctcp_bytesMarked << " bytes marked in this window\n";
        conn->emit(markingProbSignal, marked > 0 ? 1 : 0);
    }
    else if (state->gotEce) {
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
        EV_INFO << "DCTCP: end of a window of data: " << state->dctcp_bytesMarked << " of " << state->dctcp_bytesAcked
                << " bytes marked, alpha=" << state->dctcp_alpha << "\n";

        // RFC 8257 3.3.7
        state->dctcp_windEnd = state->snd_nxt;

        // RFC 8257 3.3.8
        state->dctcp_bytesAcked = state->dctcp_bytesMarked = 0;
        state->sndCwr = false;
    }
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

