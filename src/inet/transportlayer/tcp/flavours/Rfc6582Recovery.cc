//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"

namespace inet {
namespace tcp {

bool Rfc6582Recovery::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    Rfc5681Recovery rfc5681Recovery(state, conn);
    return rfc5681Recovery.isDuplicateAck(tcpHeader, payloadLength);
}

void Rfc6582Recovery::receivedAckForUnackedData(uint32_t numBytesAcked)
{
    TcpAlgorithmBase *algorithm = check_and_cast<TcpAlgorithmBase *>(conn->getTcpAlgorithmForUpdate());

    // RFC 3782, page 5:
    // "5) When an ACK arrives that acknowledges new data, this ACK could be
    // the acknowledgment elicited by the retransmission from step 2, or
    // elicited by a later retransmission.
    //
    // Full acknowledgements:
    // If this ACK acknowledges all of the data up to and including
    // "recover", then the ACK acknowledges all the intermediate
    // segments sent between the original transmission of the lost
    // segment and the receipt of the third duplicate ACK.  Set cwnd to
    // either (1) min (ssthresh, FlightSize + SMSS) or (2) ssthresh,
    // where ssthresh is the value set in step 1; this is termed
    // "deflating" the window.  (We note that "FlightSize" in step 1
    // referred to the amount of data outstanding in step 1, when Fast
    // Recovery was entered, while "FlightSize" in step 5 refers to the
    // amount of data outstanding in step 5, when Fast Recovery is
    // exited.)  If the second option is selected, the implementation is
    // encouraged to take measures to avoid a possible burst of data, in
    // case the amount of data outstanding in the network is much less
    // than the new congestion window allows.  A simple mechanism is to
    // limit the number of data packets that can be sent in response to
    // a single acknowledgement; this is known as "maxburst_" in the NS
    // simulator.  Exit the Fast Recovery procedure."
    if (seqGE(state->snd_una - 1, state->recover)) {
        uint32_t flight_size = state->snd_max - state->snd_una;
        // option (1)
        state->snd_cwnd = std::min(state->ssthresh, flight_size + state->snd_mss);
        EV_INFO << "Fast Recovery - Full ACK received: Exit Fast Recovery, setting cwnd to " << state->snd_cwnd << "\n";
        conn->emit(cwndSignal, state->snd_cwnd);

        state->lossRecovery = false;
        state->firstPartialACK = false;
        EV_INFO << "Loss Recovery terminated.\n";
    }
    else {
        // RFC 3782, page 5:
        // "Partial acknowledgements:
        // If this ACK does *not* acknowledge all of the data up to and
        // including "recover", then this is a partial ACK.  In this case,
        // retransmit the first unacknowledged segment.  Deflate the
        // congestion window by the amount of new data acknowledged by the
        // cumulative acknowledgement field.  If the partial ACK
        // acknowledges at least one SMSS of new data, then add back SMSS
        // bytes to the congestion window.  As in Step 3, this artificially
        // inflates the congestion window in order to reflect the additional
        // segment that has left the network.  Send a new segment if
        // permitted by the new value of cwnd.  This "partial window
        // deflation" attempts to ensure that, when Fast Recovery eventually
        // ends, approximately ssthresh amount of data will be outstanding
        // in the network.  Do not exit the Fast Recovery procedure (i.e.,
        // if any duplicate ACKs subsequently arrive, execute Steps 3 and 4
        // above).
        //
        // For the first partial ACK that arrives during Fast Recovery, also
        // reset the retransmit timer.  Timer management is discussed in
        // more detail in Section 4."
        EV_INFO << "Fast Recovery - Partial ACK received: retransmitting the first unacknowledged segment\n";
        // retransmit first unacknowledged segment
        conn->retransmitOneSegment(false);

        // deflate cwnd by amount of new data acknowledged by cumulative acknowledgement field
        state->snd_cwnd -= numBytesAcked;
        conn->emit(cwndSignal, state->snd_cwnd);
        EV_INFO << "Fast Recovery: deflating cwnd by amount of new data acknowledged, new cwnd=" << state->snd_cwnd << "\n";

        // if the partial ACK acknowledges at least one SMSS of new data, then add back SMSS bytes to the cwnd
        if (numBytesAcked >= state->snd_mss) {
            state->snd_cwnd += state->snd_mss;
            conn->emit(cwndSignal, state->snd_cwnd);
            EV_DETAIL << "Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";
        }

        // try to send a new segment if permitted by the new value of cwnd
        algorithm->sendData(false);

        // reset REXMIT timer for the first partial ACK that arrives during Fast Recovery
        if (state->lossRecovery) {
            if (!state->firstPartialACK) {
                state->firstPartialACK = true;
                EV_DETAIL << "First partial ACK arrived during recovery, restarting REXMIT timer.\n";
                algorithm->restartRexmitTimer();
            }
        }
    }
}

void Rfc6582Recovery::receivedDuplicateAck()
{
    TcpAlgorithmBase *algorithm = check_and_cast<TcpAlgorithmBase *>(conn->getTcpAlgorithmForUpdate());

    if (state->dupacks == state->dupthresh) {
        if (!state->lossRecovery) {
            // RFC 3782, page 4:
            // "1) Three duplicate ACKs:
            // When the third duplicate ACK is received and the sender is not
            // already in the Fast Recovery procedure, check to see if the
            // Cumulative Acknowledgement field covers more than "recover".  If
            // so, go to Step 1A.  Otherwise, go to Step 1B."
            if (state->snd_una - 1 > state->recover) {
                EV_INFO << "NewReno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": perform Fast Retransmit, and enter Fast Recovery:";

                // RFC 3782, page 4:
                // "1A) Invoking Fast Retransmit:
                // If so, then set ssthresh to no more than the value given in
                // equation 3 of [RFC2581], and record the highest sequence number
                // transmitted in the variable "recover", and go to Step 2."
                // The flight size is estimated as min(cwnd, snd_wnd).
                state->ssthresh = algorithm->calculateSsthresh(std::min(state->snd_cwnd, state->snd_wnd));
                conn->emit(ssthreshSignal, state->ssthresh);
                state->recover = (state->snd_max - 1);
                state->firstPartialACK = false;
                state->lossRecovery = true;
                EV_INFO << " set recover=" << state->recover;

                // RFC 3782, page 4:
                // "2) Entering Fast Retransmit:
                // Retransmit the lost segment and set cwnd to ssthresh plus 3 * SMSS.
                // This artificially "inflates" the congestion window by the number
                // of segments (three) that have left the network and the receiver
                // has buffered."
                state->snd_cwnd = state->ssthresh + 3 * state->snd_mss;
                conn->emit(cwndSignal, state->snd_cwnd);

                EV_DETAIL << " , cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";
                conn->retransmitOneSegment(false);

                // RFC 3782, page 5:
                // "4) Fast Recovery, continued:
                // Transmit a segment, if allowed by the new value of cwnd and the
                // receiver's advertised window."
                algorithm->sendData(false);
            }
            else {
                // RFC 3782, page 4:
                // "1B) Not invoking Fast Retransmit:
                // Do not enter the Fast Retransmit and Fast Recovery procedure.  In
                // particular, do not change ssthresh, do not go to Step 2 to
                // retransmit the "lost" segment, and do not execute Step 3 upon
                // subsequent duplicate ACKs."
                EV_INFO << "NewReno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": not invoking Fast Retransmit and Fast Recovery\n";
            }
        }
        EV_INFO << "NewReno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": TCP is already in Fast Recovery procedure\n";
    }
    else if (state->dupacks > state->dupthresh) {
        if (state->lossRecovery) {
            // RFC 3782, page 4:
            // "3) Fast Recovery:
            // In Fast Recovery, increment cwnd by SMSS for each additional
            // duplicate ACK received while in Fast Recovery.  This artificially
            // inflates the congestion window in order to reflect the additional
            // segment that has left the network."
            state->snd_cwnd += state->snd_mss;
            conn->emit(cwndSignal, state->snd_cwnd);
            EV_DETAIL << "NewReno on dupAcks > DUPTHRESH(=" << state->dupthresh << ": Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";

            // RFC 3782, page 5:
            // "4) Fast Recovery, continued:
            // Transmit a segment, if allowed by the new value of cwnd and the
            // receiver's advertised window."
            algorithm->sendData(false);
        }
    }
}

void Rfc6582Recovery::onRexmitTimeout()
{
    // RFC 3782, page 6:
    // "6) Retransmit timeouts:
    // After a retransmit timeout, record the highest sequence number
    // transmitted in the variable "recover" and exit the Fast Recovery
    // procedure if applicable."
    state->recover = (state->snd_max - 1);
    EV_INFO << "recover=" << state->recover << "\n";
    state->lossRecovery = false;
    state->firstPartialACK = false;
    EV_INFO << "Loss Recovery terminated.\n";
}

} // namespace tcp
} // namespace inet

