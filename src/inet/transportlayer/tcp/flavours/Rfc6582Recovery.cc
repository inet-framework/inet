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

Rfc6582Recovery::Rfc6582Recovery(TcpStateVariables *state, TcpConnection *conn) :
    state(check_and_cast<TcpClassicAlgorithmBaseStateVariables *>(state)), conn(conn)
{
    // RFC 6582, page 5:
    // "1)  Initialization of TCP protocol control block:
    //      When the TCP protocol control block is initialized, recover is
    //      set to the initial send sequence number."
    // The connection creates the recovery when it is established, after it
    // has chosen the initial send sequence number.
    this->state->recover = this->state->iss;
}

bool Rfc6582Recovery::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    Rfc5681Recovery rfc5681Recovery(state, conn);
    return rfc5681Recovery.isDuplicateAck(tcpHeader, payloadLength);
}

void Rfc6582Recovery::receivedAckForUnackedData(uint32_t numBytesAcked)
{
    TcpAlgorithmBase *algorithm = check_and_cast<TcpAlgorithmBase *>(conn->getTcpAlgorithmForUpdate());

    // RFC 6582, page 5:
    // "3)  Response to newly acknowledged data:
    //      ...
    //      Full acknowledgments:
    //      If this ACK acknowledges all of the data up to and including
    //      recover, then the ACK acknowledges all the intermediate segments
    //      sent between the original transmission of the lost segment and
    //      the receipt of the third duplicate ACK.  Set cwnd to either (1)
    //      min (ssthresh, max(FlightSize, SMSS) + SMSS) or (2) ssthresh,
    //      where ssthresh is the value set when fast retransmit was entered,
    //      and where FlightSize in (1) is the amount of data presently
    //      outstanding.  This is termed "deflating" the window. ...
    //      Exit the fast recovery procedure."
    if (seqGE(state->snd_una - 1, state->recover)) {
        // option (1): without Proportional Rate Reduction, option (2) can send a burst
        uint32_t flight_size = state->snd_max - state->snd_una;
        state->snd_cwnd = std::min(state->ssthresh, flight_size + state->snd_mss);
        EV_INFO << "Fast Recovery - Full ACK received: Exit Fast Recovery, setting cwnd to " << state->snd_cwnd << "\n";
        conn->emit(cwndSignal, state->snd_cwnd);
        state->lossRecovery = false;
        state->firstPartialACK = false;
        EV_INFO << "Loss Recovery terminated.\n";
    }
    else {
        // RFC 6582, page 6:
        // "Partial acknowledgments:
        //  If this ACK does *not* acknowledge all of the data up to and
        //  including recover, then this is a partial ACK.  In this case,
        //  retransmit the first unacknowledged segment.  Deflate the
        //  congestion window by the amount of new data acknowledged by the
        //  Cumulative Acknowledgment field.  If the partial ACK acknowledges
        //  at least one SMSS of new data, then add back SMSS bytes to the
        //  congestion window.  This artificially inflates the congestion
        //  window in order to reflect the additional segment that has left
        //  the network.  Send a new segment if permitted by the new value of
        //  cwnd. ...  For the first partial ACK that arrives during fast
        //  recovery, also reset the retransmit timer."
        // cwnd is not deflated: the newly acknowledged data has left the bytes
        // in flight. The new head is lost (Linux tcp_newreno_mark_lost()), and
        // its retransmission takes its place in the bytes in flight.
        EV_INFO << "Fast Recovery - Partial ACK received: retransmitting the first unacknowledged segment\n";
        conn->getRexmitQueueForUpdate()->markHeadLost();
        conn->retransmitOneSegment(false);

        // try to send a new segment if permitted by cwnd
        algorithm->sendData(false);

        // reset REXMIT timer for the first partial ACK that arrives during Fast Recovery
        if (!state->firstPartialACK) {
            state->firstPartialACK = true;
            EV_DETAIL << "First partial ACK arrived during recovery, restarting REXMIT timer.\n";
            algorithm->restartRexmitTimer();
        }
    }
}

void Rfc6582Recovery::receivedDuplicateAck()
{
    // RFC 6582, page 5:
    // "2)  Three duplicate ACKs:
    //      When the third duplicate ACK is received, the TCP sender first
    //      checks the value of recover to see if the Cumulative
    //      Acknowledgment field covers more than recover.  If so, the value
    //      of recover is incremented to the value of the highest sequence
    //      number transmitted by the TCP so far.  The TCP then enters fast
    //      retransmit (step 2 of Section 3.2 of [RFC5681]).  If not, the TCP
    //      does not enter fast retransmit and does not reset ssthresh."
    if (state->dupacks == state->dupthresh && !state->lossRecovery) {
        if (seqGreater(state->snd_una - 1, state->recover)) {
            state->recover = (state->snd_max - 1);
            state->firstPartialACK = false;
            EV_INFO << "NewReno: set recover=" << state->recover << "\n";
            Rfc5681Recovery rfc5681Recovery(state, conn);
            rfc5681Recovery.receivedDuplicateAck();
        }
        else
            EV_INFO << "NewReno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": not invoking Fast Retransmit and Fast Recovery\n";
    }
    else if (state->lossRecovery) {
        // RFC 6582, page 6: "... if any duplicate ACKs subsequently arrive,
        // execute step 4 of Section 3.2 of [RFC5681]."
        Rfc5681Recovery rfc5681Recovery(state, conn);
        rfc5681Recovery.receivedDuplicateAck();
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

