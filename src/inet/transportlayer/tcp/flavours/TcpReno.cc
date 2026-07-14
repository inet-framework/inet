//
// Copyright (C) 2004-2005 OpenSim Ltd.
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpReno.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

Register_Class(TcpReno);

TcpReno::TcpReno() : TcpTahoeRenoFamily(),
    state((TcpRenoStateVariables *&)TcpAlgorithm::state)
{
}

void TcpReno::recalculateSlowStartThreshold()
{
    // RFC 2581, page 4:
    // "When a TCP sender detects segment loss using the retransmission
    // timer, the value of ssthresh MUST be set to no more than the value
    // given in equation 3:
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (3)
    //
    // As discussed above, FlightSize is the amount of outstanding data in
    // the network."

    // set ssthresh to flight size / 2, but at least 2 SMSS
    // (the formula below practically amounts to ssthresh = cwnd / 2 most of the time)
    uint32_t flight_size = conn->getFlightSize();
    state->ssthresh = std::max(flight_size / 2, 2 * state->snd_mss);

    conn->emit(ssthreshSignal, state->ssthresh);
}

uint32_t TcpReno::prrNewlyDelivered() const
{
    // bytes newly cumulatively-acked + selectively-acked by the ACK being processed
    // (snapshot taken at the top of process_RCV_SEGMENT)
    return (uint32_t)(state->deliveredBytes - state->prrDeliveredMark);
}

void TcpReno::prrInitCwndReduction()
{
    // RFC 6937 / Linux tcp_init_cwnd_reduction(): snapshot cwnd, reset the PRR
    // counters, and reduce ssthresh via the flavour's decrease function.
    state->priorCwnd = state->snd_cwnd;
    state->prrDelivered = 0;
    state->prrOut = 0;
    recalculateSlowStartThreshold(); // sets state->ssthresh and emits ssthreshSignal
}

void TcpReno::prrCwndReduction(int newlyAckedSacked, int newlyLost, bool sndUnaAdvanced)
{
    // RFC 6937 / Linux tcp_cwnd_reduction(): proportional rate reduction. All
    // quantities are in bytes (Linux counts packets); 1 packet == snd_mss bytes.
    if (newlyAckedSacked <= 0 || state->priorCwnd == 0)
        return;

    conn->setPipe();
    int pipeNow = (int)state->pipe;
    int delta = (int)state->ssthresh - pipeNow;

    state->prrDelivered += newlyAckedSacked;

    int sndcnt;
    if (delta < 0) {
        // proportional phase: bound sending to the reduction slope
        uint64_t dividend = (uint64_t)state->ssthresh * state->prrDelivered + state->priorCwnd - 1;
        sndcnt = (int)(dividend / state->priorCwnd) - (int)state->prrOut;
    }
    else {
        // slow-start-reduction-bound phase
        sndcnt = std::max((int)state->prrDelivered - (int)state->prrOut, newlyAckedSacked);
        if (sndUnaAdvanced && newlyLost == 0)
            sndcnt += (int)state->snd_mss;
        sndcnt = std::min(delta, sndcnt);
    }
    // force at least one segment out on entering fast recovery (prrOut == 0)
    sndcnt = std::max(sndcnt, (int)(state->prrOut ? 0 : state->snd_mss));

    state->snd_cwnd = (uint32_t)std::max(0, pipeNow + sndcnt);
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_DETAIL << "PRR: pipe=" << pipeNow << " ssthresh=" << state->ssthresh
              << " prrDelivered=" << state->prrDelivered << " prrOut=" << state->prrOut
              << " sndcnt=" << sndcnt << " -> cwnd=" << state->snd_cwnd << "\n";
}

void TcpReno::prrEndCwndReduction()
{
    // RFC 6937 / Linux tcp_end_cwnd_reduction(): set cwnd to ssthresh on leaving recovery.
    state->snd_cwnd = state->ssthresh;
    conn->emit(cwndSignal, state->snd_cwnd);
    EV_INFO << "PRR: leaving fast recovery, cwnd=ssthresh=" << state->snd_cwnd << "\n";
}

void TcpReno::undoInit()
{
    // Linux tcp_init_undo(): remember the pre-reduction cwnd/ssthresh so a later
    // D-SACK (or Eifel timestamp) can restore them. Must be called BEFORE the
    // ssthresh/cwnd reduction. undoRetrans starts at -1 ("no retransmit yet"),
    // becomes >0 as retransmissions go out, and returns to 0 once every one of
    // them is confirmed spurious by a D-SACK.
    state->undoMarker = state->snd_una ? state->snd_una : 1; // nonzero marker
    state->priorSsthresh = state->ssthresh;
    state->priorCwnd = state->snd_cwnd;
    state->undoRetrans = -1;
    state->retransStampTS = 0;
}

bool TcpReno::mayUndo() const
{
    // Linux tcp_may_undo(): undo when every retransmission of the episode has been
    // D-SACKed (undoRetrans == 0). (The Eifel/timestamp packet-delayed path is a
    // planned refinement.)
    return state->undoMarker != 0 && state->undoRetrans == 0;
}

void TcpReno::undoCwndReduction()
{
    // Linux tcp_undo_cwnd_reduction(): restore cwnd and ssthresh.
    state->snd_cwnd = std::max(state->snd_cwnd, state->priorCwnd); // tcp_reno_undo_cwnd
    if (state->priorSsthresh > state->ssthresh)
        state->ssthresh = state->priorSsthresh;
    state->undoMarker = 0;
    conn->emit(cwndSignal, state->snd_cwnd);
    conn->emit(ssthreshSignal, state->ssthresh);
    EV_INFO << "Undoing spurious cwnd reduction (D-SACK): cwnd=" << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";
}

void TcpReno::processRexmitTimer(TcpEventCode& event)
{
    TcpTahoeRenoFamily::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // After REXMIT timeout TCP Reno should start slow start with snd_cwnd = snd_mss.
    //
    // If calling "retransmitData();" there is no rexmit limitation (bytesToSend > snd_cwnd)
    // therefore "sendData();" has been modified and is called to rexmit outstanding data.
    //
    // RFC 2581, page 5:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."

    // begin Slow Start (RFC 2581)
    recalculateSlowStartThreshold();
    state->snd_cwnd = state->snd_mss;

    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;

    conn->retransmitOneSegment(true);
}

void TcpReno::segmentsAcked(uint32_t fromSeq, uint32_t toSeq)
{
    // Adaptive reordering: if this cumulatively-acked segment was never
    // retransmitted yet sits below already-SACKed data, it was merely reordered
    // (not lost) -- grow the learned reordering degree so it stops causing
    // spurious fast retransmits.
    if (state->adaptiveReorderingEnabled && state->sack_enabled) {
        const TcpSackRexmitQueue *rq = conn->getRexmitQueue();
        if (rq != nullptr && rq->getQueueLength() > 0
            && seqLE(rq->getBufferStartSeq(), fromSeq) && seqLess(fromSeq, rq->getBufferEndSeq())
            && rq->getRegion(fromSeq).transmitCount <= 1)
        {
            conn->checkSackReordering(fromSeq);
        }
    }
}

void TcpReno::receivedDataAck(uint32_t firstSeqAcked)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

    if (state->lossUndoEnabled && mayUndo()) {
        // RFC 2883: every retransmission of this episode has been D-SACKed, so the
        // recovery was spurious -- restore cwnd/ssthresh and reopen (no deflation).
        undoCwndReduction();
        state->lossRecovery = false;
    }
    else if (state->prrEnabled && state->lossRecovery) {
        // RFC 6937: while in fast recovery, PRR sizes cwnd from delivered bytes
        // (no slow start / congestion avoidance growth). The deflation to ssthresh
        // happens at recovery exit (prrEndCwndReduction, below).
        prrCwndReduction((int)prrNewlyDelivered(), 0, true /*snd_una advanced*/);
    }
    else if (state->dupacks >= state->reordering) {
        //
        // Perform Fast Recovery: set cwnd to ssthresh (deflating the window).
        //
        EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;

        conn->emit(cwndSignal, state->snd_cwnd);
    }
    else {
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
        if (performSsCa) {
            // If ECN is not enabled or if ECN is enabled and received multiple ECE-Acks in
            // less than RTT, then perform slow start and congestion avoidance.

            if (state->snd_cwnd < state->ssthresh) {
                // RFC 5681 slow start (Eq 2): cwnd += min(N, SMSS) per ACK -- at
                // most one SMSS, keeping textbook Reno growth. The only change is
                // the cwnd-limited gate (RFC 5681 principle, Linux tcp_is_cwnd_
                // limited): grow only while the sender actually fills the window
                // (cwnd < 2 * max_packets_out, in segments); an application-limited
                // flow that never fills cwnd must not inflate it. (Byte-counting/
                // ABC growth is deliberately NOT adopted here -- that is a
                // different spec, RFC 3465, used only by the Linux-modelled
                // TcpCubic.)
                if ((state->snd_cwnd / state->snd_mss) < 2 * state->maxPacketsOut) {
                    state->snd_cwnd += state->snd_mss;
                    conn->emit(cwndSignal, state->snd_cwnd);
                    conn->emit(ssthreshSignal, state->ssthresh);
                }
                EV_INFO << "cwnd <= ssthresh: Slow Start: cwnd=" << state->snd_cwnd << "\n";
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
            if (state->prrEnabled)
                prrEndCwndReduction(); // RFC 6937: snd_cwnd = ssthresh on exit
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

void TcpReno::receivedDuplicateAck()
{
    TcpTahoeRenoFamily::receivedDuplicateAck();

    // In RACK mode a fast retransmit is triggered by time-based loss detection
    // rather than by counting duplicate ACKs.
    bool rackTrigger = (state->lossDetectionMode == 1 && state->sack_enabled
                        && !state->lossRecovery
                        && conn->getRexmitQueue()->getTotalAmountOfLostBytes() > 0
                        && (state->recoveryPoint == 0 || seqGE(state->snd_una, state->recoveryPoint)));

    if (state->dupacks == state->reordering || rackTrigger) {
        EV_INFO << "Reno on dupAcks == DUPTHRESH(=" << state->reordering << ": perform Fast Retransmit, and enter Fast Recovery:";

        if (state->sack_enabled) {
            // RFC 3517, page 6: "When a TCP sender receives the duplicate ACK corresponding to
            // DupThresh ACKs, the scoreboard MUST be updated with the new SACK
            // information (via Update ()).  If no previous loss event has occurred
            // on the connection or the cumulative acknowledgment point is beyond
            // the last value of RecoveryPoint, a loss recovery phase SHOULD be
            // initiated, per the fast retransmit algorithm outlined in [RFC2581].
            // The following steps MUST be taken:
            //
            // (1) RecoveryPoint = HighData
            //
            // When the TCP sender receives a cumulative ACK for this data octet
            // the loss recovery phase is terminated."

            // RFC 3517, page 8: "If an RTO occurs during loss recovery as specified in this document,
            // RecoveryPoint MUST be set to HighData.  Further, the new value of
            // RecoveryPoint MUST be preserved and the loss recovery algorithm
            // outlined in this document MUST be terminated.  In addition, a new
            // recovery phase (as described in section 5) MUST NOT be initiated
            // until HighACK is greater than or equal to the new value of
            // RecoveryPoint."
            if (state->recoveryPoint == 0 || seqGE(state->snd_una, state->recoveryPoint)) { // HighACK = snd_una
                state->recoveryPoint = state->snd_max; // HighData = snd_max
                state->lossRecovery = true;
                EV_DETAIL << " recoveryPoint=" << state->recoveryPoint;
            }
        }
        // RFC 2581, page 5:
        // "After the fast retransmit algorithm sends what appears to be the
        // missing segment, the "fast recovery" algorithm governs the
        // transmission of new data until a non-duplicate ACK arrives.
        // (...) the TCP sender can continue to transmit new
        // segments (although transmission must continue using a reduced cwnd)."

        // capture the undo context BEFORE reducing ssthresh/cwnd (RFC 2883/3522)
        if (state->lossUndoEnabled)
            undoInit();

        // enter Fast Recovery
        if (state->prrEnabled) {
            // RFC 6937 Proportional Rate Reduction: no cwnd inflation. Size cwnd
            // with prrOut==0 (forces at least one segment out for the fast
            // retransmit), retransmit, then fill the pipe up to the PRR cwnd.
            prrInitCwndReduction();
            prrCwndReduction((int)prrNewlyDelivered(), 0, false /*snd_una not advanced*/);
            EV_DETAIL << " PRR fast recovery: priorCwnd=" << state->priorCwnd
                      << ", ssthresh=" << state->ssthresh << ", cwnd=" << state->snd_cwnd << "\n";

            conn->retransmitOneSegment(false);

            if (state->lossRecovery) {
                restartRexmitTimer();
                conn->setPipe();
                if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss)
                    conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
            }
        }
        else {
            recalculateSlowStartThreshold();
            // "set cwnd to ssthresh plus 3 * SMSS." (RFC 2581)
            state->snd_cwnd = state->ssthresh + 3 * state->snd_mss; // 20051129 (1)

            conn->emit(cwndSignal, state->snd_cwnd);

            EV_DETAIL << " set cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";

            // Fast Retransmission: retransmit missing segment without waiting
            // for the REXMIT timer to expire
            conn->retransmitOneSegment(false);

            // Do not restart REXMIT timer.
            // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
            // Resetting the REXMIT timer is discussed in RFC 2582/3782 (NewReno) and RFC 2988.

            if (state->sack_enabled) {
                // RFC 3517, page 7: "(4) Run SetPipe ()
                //
                // Set a "pipe" variable  to the number of outstanding octets
                // currently "in the pipe"; this is the data which has been sent by
                // the TCP sender but for which no cumulative or selective
                // acknowledgment has been received and the data has not been
                // determined to have been dropped in the network.  It is assumed
                // that the data is still traversing the network path."
                conn->setPipe();
                // RFC 3517, page 7: "(5) In order to take advantage of potential additional available
                // cwnd, proceed to step (C) below."
                if (state->lossRecovery) {
                    // RFC 3517, page 9: "Therefore we give implementers the latitude to use the standard
                    // [RFC2988] style RTO management or, optionally, a more careful variant
                    // that re-arms the RTO timer on each retransmission that is sent during
                    // recovery MAY be used.  This provides a more conservative timer than
                    // specified in [RFC2988], and so may not always be an attractive
                    // alternative.  However, in some cases it may prevent needless
                    // retransmissions, go-back-N transmission and further reduction of the
                    // congestion window."
                    // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
                    EV_INFO << "Retransmission sent during recovery, restarting REXMIT timer.\n";
                    restartRexmitTimer();

                    // RFC 3517, page 7: "(C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
                    // segments as follows:"
                    if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss) // Note: Typecast needed to avoid prohibited transmissions
                        conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
                }
            }
        }

        // try to transmit new segments (RFC 2581)
        sendData(false);
    }
    else if (state->dupacks > state->reordering) {
        if (state->prrEnabled && state->lossRecovery) {
            // RFC 6937: each additional dup ACK carrying new SACK info sizes cwnd
            // by PRR rather than inflating it by one SMSS.
            prrCwndReduction((int)prrNewlyDelivered(), 0, false /*snd_una not advanced*/);
            conn->setPipe();
            if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss)
                conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
        }
        else {
            //
            // Reno: For each additional duplicate ACK received, increment cwnd by SMSS.
            // This artificially inflates the congestion window in order to reflect the
            // additional segment that has left the network
            //
            state->snd_cwnd += state->snd_mss;
            EV_DETAIL << "Reno on dupAcks > DUPTHRESH(=" << state->dupthresh << ": Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";

            conn->emit(cwndSignal, state->snd_cwnd);
        }

        // Note: Steps (A) - (C) of RFC 3517, page 7 ("Once a TCP is in the loss recovery phase the following procedure MUST be used for each arriving ACK")
        // should not be used here (non-PRR)!

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
}

} // namespace tcp
} // namespace inet

