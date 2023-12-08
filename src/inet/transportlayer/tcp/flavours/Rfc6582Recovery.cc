//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

bool Rfc6582Recovery::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    Rfc5681Recovery rfc5681Recovery(state, conn);
    return rfc5681Recovery.isDuplicateAck(tcpHeader, payloadLength);
}

void Rfc6582Recovery::receivedAckForUnackedData(uint32_t numBytesAcked)
{
    //"
    // 3.2. Specification
    //"
    if (state->lossRecovery) {
        //"
        // 3) Response to newly acknowledged data:
        //    Step 6 of [RFC5681] specifies the response to the next ACK that
        //    acknowledges previously unacknowledged data.  When an ACK arrives
        //    that acknowledges new data, this ACK could be the acknowledgment
        //    elicited by the initial retransmission from fast retransmit, or
        //    elicited by a later retransmission.  There are two cases:
        //"
        if (seqGE(state->snd_una - 1, state->recover)) {
            //"
            // Full acknowledgments:
            // If this ACK acknowledges all of the data up to and including
            // recover, then the ACK acknowledges all the intermediate segments
            // sent between the original transmission of the lost segment and
            // the receipt of the third duplicate ACK.  Set cwnd to either (1)
            // min (ssthresh, max(FlightSize, SMSS) + SMSS) or (2) ssthresh,
            // where ssthresh is the value set when fast retransmit was entered,
            // and where FlightSize in (1) is the amount of data presently
            // outstanding.  This is termed "deflating" the window.  If the
            // second option is selected, the implementation is encouraged to
            // take measures to avoid a possible burst of data, in case the
            // amount of data outstanding in the network is much less than the
            // new congestion window allows.  A simple mechanism is to limit the
            // number of data packets that can be sent in response to a single
            // acknowledgment.  Exit the fast recovery procedure.
            //"
            state->snd_cwnd = state->ssthresh; // use option (2)
            conn->emit(cwndSignal, state->ssthresh);
            state->lossRecovery = false;
            state->firstPartialACK = false;
            EV_INFO << "Loss recovery terminated" << EV_ENDL;
        }
        else {
            //"
            // Partial acknowledgments:
            // If this ACK does *not* acknowledge all of the data up to and
            // including recover, then this is a partial ACK.  In this case,
            // retransmit the first unacknowledged segment.  Deflate the
            // congestion window by the amount of new data acknowledged by the
            // Cumulative Acknowledgment field.  If the partial ACK acknowledges
            // at least one SMSS of new data, then add back SMSS bytes to the
            // congestion window.  This artificially inflates the congestion
            // window in order to reflect the additional segment that has left
            // the network.
            //"
            conn->retransmitOneSegment(false);

            //"
            // Send a new segment if permitted by the new value of
            // cwnd.  This "partial window deflation" attempts to ensure that,
            // when fast recovery eventually ends, approximately ssthresh amount
            // of data will be outstanding in the network.  Do not exit the fast
            // recovery procedure (i.e., if any duplicate ACKs subsequently
            // arrive, execute step 4 of Section 3.2 of [RFC5681]).
            //"
            conn->sendData(state->snd_cwnd);

            //"
            // For the first partial ACK that arrives during fast recovery, also
            // reset the retransmit timer.  Timer management is discussed in
            // more detail in Section 4.
            //"
            if (!state->firstPartialACK) {
                state->firstPartialACK = true;
                EV_DETAIL << "First partial ACK arrived during recovery, restarting REXMIT timer.\n";
                conn->getTcpAlgorithmForUpdate()->restartRexmitTimer();
            }
        }
        //"
        // 4) Retransmit timeouts:
        //    After a retransmit timeout, record the highest sequence number
        //    transmitted in the variable recover, and exit the fast recovery
        //    procedure if applicable.
        //
        // Step 2 above specifies a check that the Cumulative Acknowledgment
        // field covers more than recover.  Because the acknowledgment field
        // contains the sequence number that the sender next expects to receive,
        // the acknowledgment "ack_number" covers more than recover when
        //
        //   ack_number - 1 > recover;
        //
        // i.e., at least one byte more of data is acknowledged beyond the
        // highest byte that was outstanding when fast retransmit was last
        // entered.
        //
        // Note that in step 3 above, the congestion window is deflated after a
        // partial acknowledgment is received.  The congestion window was likely
        // to have been inflated considerably when the partial acknowledgment
        // was received.  In addition, depending on the original pattern of
        // packet losses, the partial acknowledgment might acknowledge nearly a
        // window of data.  In this case, if the congestion window was not
        // deflated, the data sender might be able to send nearly a window of
        // data back-to-back.
        //
        // This document does not specify the sender's response to duplicate
        // ACKs when the fast retransmit/fast recovery algorithm is not invoked.
        // This is addressed in other documents, such as those describing the
        // Limited Transmit procedure [RFC3042].  This document also does not
        // address issues of adjusting the duplicate acknowledgment threshold,
        // but assumes the threshold specified in the IETF standards; the
        // current standard is [RFC5681], which specifies a threshold of three
        // duplicate acknowledgments.
        //
        // As a final note, we would observe that in the absence of the SACK
        // option, the data sender is working from limited information.  When
        // the issue of recovery from multiple dropped packets from a single
        // window of data is of particular importance, the best alternative
        // would be to use the SACK option.
        //"
    }
}

void Rfc6582Recovery::receivedDuplicateAck()
{
    //"
    // 3.2.  Specification
    // ...
    // 2) Three duplicate ACKs:
    //    When the third duplicate ACK is received, the TCP sender first
    //    checks the value of recover to see if the Cumulative
    //    Acknowledgment field covers more than recover.  If so, the value
    //    of recover is incremented to the value of the highest sequence
    //    number transmitted by the TCP so far.  The TCP then enters fast
    //    retransmit (step 2 of Section 3.2 of [RFC5681]).  If not, the TCP
    //    does not enter fast retransmit and does not reset ssthresh.
    //"
    if (state->dupacks == state->dupthresh) {
        conn->getRexmitQueueForUpdate()->markHeadLost(); // update for flight size calculation
        if (!state->lossRecovery) {
            if (seqGreater(state->snd_una - 1, state->recover)) {
                state->recover = state->snd_max - 1;

                Rfc5681Recovery rfc5681Recovery(state, conn);
                rfc5681Recovery.receivedDuplicateAck(); // TODO be more specific

                // entering fast retransmit means starting the loss recovery phase
                state->lossRecovery = true;
                state->firstPartialACK = false;
            }
        }
        else
            conn->sendData(state->snd_cwnd); // TODO why?
    }
    else {
        Rfc5681Recovery rfc5681Recovery(state, conn);
        rfc5681Recovery.receivedDuplicateAck(); // TODO be more specific
    }
}

} // namespace tcp
} // namespace inet

