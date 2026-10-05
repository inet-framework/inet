//
// Copyright (C) 2004-2005 OpenSim Ltd.
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpReno.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpReno);

TcpReno::TcpReno() : TcpClassicAlgorithmBase(),
    state((TcpRenoStateVariables *&)TcpAlgorithm::state)
{
}

ITcpCongestionControl *TcpReno::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

ITcpRecovery *TcpReno::createRecovery()
{
    return new Rfc5681Recovery(state, conn);
}

bool TcpReno::processEce()
{
    bool performSsCa = true; // Stands for: "perform slow start and congestion avoidance"
    if (state && state->ect && state->gotEce) {
        // halve cwnd and reduce ssthresh and do not increase cwnd (rfc-3168, page 18):
        //   If the sender receives an ECN-Echo (ECE) ACK
        // packet (that is, an ACK packet with the ECN-Echo flag set in the TCP
        // header), then the sender knows that congestion was encountered in the
        // network on the path from the sender to the receiver.  The indication
        // of congestion should be treated just as a congestion loss in non-
        // ECN-Capable TCP. That is, the TCP source halves the congestion window
        // "cwnd" and reduces the slow start threshold "ssthresh".  The sending
        // TCP SHOULD NOT increase the congestion window in response to the
        // receipt of an ECN-Echo ACK packet.
        // ...
        //   The value of the congestion window is bounded below by a value of one MSS.
        // ...
        //   TCP should not react to congestion indications more than once every
        // window of data (or more loosely, more than once every round-trip
        // time). That is, the TCP sender's congestion window should be reduced
        // only once in response to a series of dropped and/or CE packets from a
        // single window of data.  In addition, the TCP source should not decrease
        // the slow-start threshold, ssthresh, if it has been decreased
        // within the last round trip time.
        if (simTime() - state->eceReactionTime > state->srtt) {
            state->ssthresh = state->snd_cwnd / 2;
            state->snd_cwnd = std::max(state->snd_cwnd / 2, uint32_t(1));
            state->sndCwr = true;
            performSsCa = false;
            EV_INFO << "ssthresh = cwnd/2: received ECN-Echo ACK... new ssthresh = "
                    << state->ssthresh << "\n";
            EV_INFO << "cwnd /= 2: received ECN-Echo ACK... new cwnd = "
                    << state->snd_cwnd << "\n";

            // rfc-3168 page 18:
            // The sending TCP MUST reset the retransmit timer on receiving
            // the ECN-Echo packet when the congestion window is one.
            if (state->snd_cwnd == 1) {
                restartRexmitTimer();
                EV_INFO << "cwnd = 1... reset retransmit timer.\n";
            }
            state->eceReactionTime = simTime();
            conn->emit(cwndSignal, state->snd_cwnd);
            conn->emit(ssthreshSignal, state->ssthresh);
        }
        else
            EV_INFO << "multiple ECN-Echo ACKs in less than rtt... no ECN reaction\n";
        state->gotEce = false;
    }
    return !performSsCa;
}

void TcpReno::ackProcessed(bool inFastRecovery)
{
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
}

} // namespace tcp
} // namespace inet

