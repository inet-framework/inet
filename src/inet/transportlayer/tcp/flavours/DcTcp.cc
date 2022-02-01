//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/DcTcp.h"

#include <algorithm> // min,max

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
}

void DcTcp::receivedDataAck(uint32_t firstSeqAcked)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

    if (state->dupacks >= state->dupthresh) {
        //
        // Perform Fast Recovery: set cwnd to ssthresh (deflating the window).
        //
        EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;

        conn->emit(cwndSignal, state->snd_cwnd);
    }
    else {
        bool performSsCa = true; // Stands for: "perform slow start and congestion avoidance"
        if (state && state->ect) {
            // RFC 8257 3.3.1
            uint32_t bytes_acked = state->snd_una - firstSeqAcked;

            // bool cut = false; TODO unused?

            // RFC 8257 3.3.2
            state->dctcp_bytesAcked += bytes_acked;

            // RFC 8257 3.3.3
            if (state->gotEce) {
                state->dctcp_bytesMarked += bytes_acked;
                conn->emit(markingProbSignal, 1);
            }
            else {
                conn->emit(markingProbSignal, 0);
            }

            // RFC 8257 3.3.4
            if (state->snd_una > state->dctcp_windEnd) {

                if (state->dctcp_bytesMarked) {
                    // cut = true;  TODO unused?
                }

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
            if ((state->dctcp_bytesMarked && !state->sndCwr)) {

                performSsCa = false;
                state->sndCwr = true;

                // RFC 8257 3.3.9
                state->snd_cwnd = state->snd_cwnd * (1 - state->dctcp_alpha / 2);

                conn->emit(cwndSignal, state->snd_cwnd);

                uint32_t flight_size = std::min(state->snd_cwnd, state->snd_wnd); // FIXME - Does this formula computes the amount of outstanding data?
                state->ssthresh = std::max(3 * flight_size / 4, 2 * state->snd_mss);

                conn->emit(ssthreshSignal, state->ssthresh);
            }
        }

        if (performSsCa) {
            // If ECN is not enabled or if ECN is enabled and received multiple ECE-Acks in
            // less than RTT, then perform slow start and congestion avoidance.

            if (state->snd_cwnd < state->ssthresh) {
                EV_INFO << "cwnd <= ssthresh: Slow Start: increasing cwnd by one SMSS bytes to ";

                // perform Slow Start. RFC 2581: "During slow start, a TCP increments cwnd
                // by at most SMSS bytes for each ACK received that acknowledges new data."
                state->snd_cwnd += state->snd_mss;

                conn->emit(cwndSignal, state->snd_cwnd);
                conn->emit(ssthreshSignal, state->ssthresh);

                EV_INFO << "cwnd=" << state->snd_cwnd << "\n";
            }
            else {
                // perform Congestion Avoidance (RFC 2581)
                uint32_t incr = state->snd_mss * state->snd_mss / state->snd_cwnd;

                if (incr == 0)
                    incr = 1;

                state->snd_cwnd += incr;

                conn->emit(cwndSignal, state->snd_cwnd);
                conn->emit(ssthreshSignal, state->ssthresh);

                //
                // Note: some implementations use extra additive constant mss / 8 here
                // which is known to be incorrect (RFC 2581 p5)
                //
                // Note 2: RFC 3465 (experimental) "Appropriate Byte Counting" (ABC)
                // would require maintaining a bytes_acked variable here which we don't do
                //

                EV_INFO << "cwnd > ssthresh: Congestion Avoidance: increasing cwnd linearly, to " << state->snd_cwnd << "\n";
            }
        }
    }

    if (state->sack_enabled && state->lossRecovery) {
        // RFC 3517, page 7: "Once a TCP is in the loss recovery phase the following procedure MUST
        // be used for each arriving ACK:
        //
        // (A) An incoming cumulative ACK for a sequence number greater than
        // RecoveryPoint signals the end of loss recovery and the loss
        // recovery phase MUST be terminated.  Any information contained in
        // the scoreboard for sequence numbers greater than the new value of
        // HighACK SHOULD NOT be cleared when leaving the loss recovery
        // phase."
        if (seqGE(state->snd_una, state->recoveryPoint)) {
            EV_INFO << "Loss Recovery terminated.\n";
            state->lossRecovery = false;
        }
        // RFC 3517, page 7: "(B) Upon receipt of an ACK that does not cover RecoveryPoint the
        // following actions MUST be taken:
        //
        // (B.1) Use Update () to record the new SACK information conveyed
        // by the incoming ACK.
        //
        // (B.2) Use SetPipe () to re-calculate the number of octets still
        // in the network."
        else {
            // update of scoreboard (B.1) has already be done in readHeaderOptions()
            conn->setPipe();

            // RFC 3517, page 7: "(C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
            // segments as follows:"
            if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss) // Note: Typecast needed to avoid prohibited transmissions
                conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
        }
    }

    // RFC 3517, pages 7 and 8: "5.1 Retransmission Timeouts
    // (...)
    // If there are segments missing from the receiver's buffer following
    // processing of the retransmitted segment, the corresponding ACK will
    // contain SACK information.  In this case, a TCP sender SHOULD use this
    // SACK information when determining what data should be sent in each
    // segment of the slow start.  The exact algorithm for this selection is
    // not specified in this document (specifically NextSeg () is
    // inappropriate during slow start after an RTO).  A relatively
    // straightforward approach to "filling in" the sequence space reported
    // as missing should be a reasonable approach."
    sendData(false);
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

