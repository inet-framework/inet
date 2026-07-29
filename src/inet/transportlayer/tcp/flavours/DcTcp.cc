//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/DcTcp.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"
#include "inet/transportlayer/tcp/Tcp.h"

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
    state->ecnMarkAll = true;
}

bool DcTcp::processEce(uint32_t numBytesAcked)
{
    // DCTCP replaces RFC 3168's once-per-RTT halving with a reduction proportional
    // to the fraction of marked bytes (RFC 8257 section 3.3), so this is where its
    // congestion response belongs: returning true tells the shared ACK path that the
    // window has been adjusted and must not also grow for this ACK -- the role the
    // old fork's "performSsCa" flag played.
    if (!state || !state->ect)
        return false;

    // RFC 8257 3.3.2
    state->dctcp_bytesAcked += numBytesAcked;

    // RFC 8257 3.3.3. AccECN: when this connection negotiated
    // AccECN, use the AccECN option's byte-exact CE evidence (deliveredCeBytes,
    // G6/G7) instead of RFC 8257's boolean gotEce-gated approximation -- AccECN
    // already gives the precise number of CE-marked bytes the peer reported, so
    // the "mark this whole round's bytes_acked if ECE was ever seen" approximation
    // isn't needed in this mode. gotEce itself is never set true for an AccECN
    // connection in the first place (the foundation-fix guard on eceBit consumption
    // -- see TcpConnectionRcvSegment.cc's processAckInEstabEtc()), so the two
    // branches below are naturally mutually exclusive per connection, exactly
    // mirroring how the two ECN modes are already mutually exclusive at negotiation
    // time elsewhere in this workstream.
    //
    // deliveredCeBytes/deliveredCePkts are cumulative (updated once per ACK in
    // processAckInEstabEtc()'s ACE block, which runs AFTER this function for the
    // very same segment -- see that block's own comment) -- the two "Mark" fields
    // snapshot them the same way prrDeliveredMark already does for deliveredBytes,
    // so this round's increment is picked up on the NEXT call, one ACK later. That
    // one-ACK lag is immaterial here: DCTCP.Alpha is a windowed EWMA over many ACKs
    // (RFC 8257 3.3.4-3.3.6 below), not a per-ACK-exact quantity, and no CE byte is
    // ever lost or double-counted -- each one is picked up on the very next round.
    //
    // Lens selection is per-CONNECTION, latched, not per-round: once
    // dctcp_accEcnOptionSeen is set (the first time this connection ever sees a
    // real AccECN option -- state->accEcnOptionCebDeltaValid, set by
    // readHeaderOptions() earlier in this same segment's processing, so unlike
    // deliveredCeBytes this flag is NOT lagged), every later round uses the
    // byte-exact deliveredCeBytes delta exclusively, even in rounds where no
    // option happened to arrive (deliveredCeBytes correctly contributes 0 for
    // those -- CEB is cumulative, so the NEXT option's delta automatically covers
    // any gap rounds, per its own design). A per-round choice ("prefer bytes when
    // nonzero, else packets") must NOT be used: a gap round would be attributed
    // to the packet-count estimate, and then the FOLLOWING option's cumulative
    // delta would re-include that same gap round's bytes, double-counting them. Before the option is ever seen at all, the packet-count
    // estimate (deliveredCePkts * snd_mss, the ACE-only estimate) is used instead
    // of reporting zero.
    if (state->accEcnNegotiated) {
        if (state->accEcnOptionCebDeltaValid)
            state->dctcp_accEcnOptionSeen = true;

        uint32_t marked;
        if (state->dctcp_accEcnOptionSeen) {
            marked = state->deliveredCeBytes - state->dctcp_deliveredCeBytesMark;
            state->dctcp_deliveredCeBytesMark = state->deliveredCeBytes;
        }
        else {
            uint32_t cePktsThisRound = state->deliveredCePkts - state->dctcp_deliveredCePktsMark;
            state->dctcp_deliveredCePktsMark = state->deliveredCePkts;
            marked = cePktsThisRound * state->snd_mss;
        }
        EV_INFO << "DcTcp AccECN CE-byte accounting: lens=" << (state->dctcp_accEcnOptionSeen ? "bytes" : "packets")
                << " marked=" << marked << " dctcp_bytesMarked=" << (state->dctcp_bytesMarked + marked) << "\n";
        if (marked > 0) {
            state->dctcp_bytesMarked += marked;
            conn->emit(markingProbSignal, 1);
        }
        else {
            conn->emit(markingProbSignal, 0);
        }
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

