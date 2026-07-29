//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

namespace inet {
namespace tcp {

bool Rfc6675Recovery::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    //"
    // For the purposes of this specification, we define a "duplicate
    // acknowledgment" as a segment that arrives carrying a SACK block that
    // identifies previously unacknowledged and un-SACKed octets between
    // HighACK and HighData.  Note that an ACK which carries new SACK data
    // is counted as a duplicate acknowledgment under this definition even
    // if it carries new data, changes the advertised window, or moves the
    // cumulative acknowledgment point, which is different from the
    // definition of duplicate acknowledgment in [RFC5681].
    //"

    // TODO unfortunately these values are wrong, see other comment where they are set
    // could be something like return state->addedSackedBytes > 0;
//    return state->sackedBytes != state->sackedBytes_old;
    return state->snd_una == tcpHeader->getAckNo() && payloadLength == 0 && state->snd_una != state->snd_max;

}

//"
// 5. Algorithm Details
//    Upon the receipt of any ACK containing SACK information, the
//    scoreboard MUST be updated via the Update () routine.
// implemented in processSACKOption()
//
//    If the incoming ACK is a cumulative acknowledgment, the TCP MUST
//    reset DupAcks to zero.
// implemented in processSegment1stThru8th() and Rfc6675Recovery::processAckInEstabEtc()
//"

void Rfc6675Recovery::stepA()
{
    //"
    // (A) An incoming cumulative ACK for a sequence number greater than
    //     RecoveryPoint signals the end of loss recovery, and the loss
    //     recovery phase MUST be terminated.  Any information contained in
    //     the scoreboard for sequence numbers greater than the new value of
    //     HighACK SHOULD NOT be cleared when leaving the loss recovery
    //     phase.
    //"
    if (seqGE(state->snd_una, state->recoveryPoint)) {
        state->lossRecovery = false;
        if (state->prrEnabled)
            prrEndCwndReduction(); // RFC 6937: deflate to ssthresh on leaving recovery
        conn->getRexmitQueueForUpdate()->discardUpTo(state->snd_una);
    }
}

void Rfc6675Recovery::stepB()
{
    //"
    // (B) Upon receipt of an ACK that does not cover RecoveryPoint, the
    //     following actions MUST be taken:
    //"
    if (seqLE(state->snd_una, state->recoveryPoint)) {
        //"
        // (B.1) Use Update () to record the new SACK information conveyed
        //       by the incoming ACK.
        // implemented by processSACKOption()
        //"

        //"
        // (B.2) Use SetPipe () to re-calculate the number of octets still
        //       in the network.
        //"
        setPipe();
    }
}

void Rfc6675Recovery::stepC()
{
    //"
    // (C) If cwnd - pipe >= 1 SMSS, the sender SHOULD transmit one or more
    //     segments as follows:
    //"
    // "1 SMSS" is compared at the size segments are actually cut to (the
    // options-adjusted effective MSS): Linux's equivalent gate is in PACKETS
    // (tcp_packets_in_flight < snd_cwnd), so a PRR budget of exactly one
    // 1000-byte segment must not be swallowed by the 12-byte timestamp
    // overhead (client-ack-dropped-then-recovery pins the second lost
    // segment going out in the same recovery-entry burst).
    while ((int32_t)state->snd_cwnd - (int32_t)state->pipe
           >= (int32_t)(state->snd_effmss > 0 ? state->snd_effmss : state->snd_mss)) {
        //"
        // (C.1) The scoreboard MUST be queried via NextSeg () for the
        //       sequence number range of the next segment to transmit (if
        //       any), and the given segment sent.  If NextSeg () returns
        //       failure (no data to send), return without sending anything
        //       (i.e., terminate steps C.1 -- C.5).
        //"
        uint32_t seqNum;
        if (!nextSeg(seqNum))
            break;

        //"
        // (C.2) If any of the data octets sent in (C.1) are below HighData,
        //       HighRxt MUST be set to the highest sequence number of the
        //       retransmitted segment unless NextSeg () rule (4) was
        //       invoked for this retransmission.
        //"
        if (seqLess(seqNum, state->snd_max))
            state->highRxt = seqNum + state->snd_mss;

        //"
        // (C.3) If any of the data octets sent in (C.1) are above HighData,
        //       HighData must be updated to reflect the transmission of
        //       previously unsent data.
        //"
        if (seqGreater(seqNum, state->snd_max)) {
            state->snd_max = seqNum + state->snd_mss;
            conn->emit(sndMaxSignal, state->snd_max);
        }

        if (seqLE(seqNum + state->snd_mss, state->snd_una + state->snd_wnd)) {
            state->snd_nxt = seqNum;
            uint32_t sentBytes = conn->sendSegment(state->snd_mss);

            // RFC 6937 accounting: sendSegment() is called here DIRECTLY (not via
            // sendData()/retransmitOneSegment()), so the dataSent()/segmentRetransmitted()
            // callbacks that feed prrOut never fire for these sends. Count them here, or
            // prrOut stays 0 and PRR's sndcnt = prrDelivered - prrOut over-sends.
            if (state->prrEnabled && state->lossRecovery)
                state->prrOut += sentBytes;

            //"
            // (C.4) The estimate of the amount of data outstanding in the
            //       network must be updated by incrementing pipe by the number
            //       of octets transmitted in (C.1).
            //"
            state->pipe += sentBytes;
        }
        else
            break;

        //"
        // (C.5) If cwnd - pipe >= 1 SMSS, return to (C.1)
        //"
    }
}

void Rfc6675Recovery::receivedAckForUnackedData(uint32_t numBytesAcked)
{
    ASSERT(state->lossRecovery);
    //"
    // Once a TCP is in the loss recovery phase, the following procedure
    // MUST be used for each arriving ACK:
    //"
    // RFC 6937: while in fast recovery PRR sizes cwnd from the bytes this ACK
    // delivered, instead of the classic inflate-per-dupack / deflate-on-exit.
    // Runs before stepA so a recovery-ending ACK still deflates to ssthresh there.
    if (state->prrEnabled && state->lossRecovery)
        prrCwndReduction((int)prrNewlyDelivered(), 0, true /* snd_una advanced */);

    stepA();
    stepB();
    stepC();
    //"
    // Note that steps (A) and (C) can potentially send a burst of
    // back-to-back segments into the network if the incoming cumulative
    // acknowledgment is for more than SMSS octets of data, or if incoming
    // SACK blocks indicate that more than SMSS octets of data have been
    // lost in the second half of the window.
    //"
}

void Rfc6675Recovery::step4()
{
    //"
    // (4) Invoke fast retransmit and enter loss recovery as follows:
    //"
    state->lossRecovery = true;

    //"
    // (4.1) RecoveryPoint = HighData
    //       When the TCP sender receives a cumulative ACK for this data
    //       octet, the loss recovery phase is terminated.
    //"
    state->recoveryPoint = state->snd_max;

    //"
    // (4.2) ssthresh = cwnd = (FlightSize / 2)
    //       The congestion window (cwnd) and slow start threshold
    //       (ssthresh) are reduced to half of FlightSize per [RFC5681].
    //       Additionally, note that [RFC5681] requires that any
    //       segments sent as part of the Limited Transmit mechanism not
    //       be counted in FlightSize for the purpose of the above
    //       equation.
    //"
    // RFC 2883/3522: capture the undo context BEFORE the reduction below, so a
    // later D-SACK proving the retransmission spurious can restore cwnd/ssthresh.
    if (state->lossUndoEnabled)
        undoInit();

    // Reduce cwnd/ssthresh per the connection's congestion-control flavour (Linux
    // icsk_ca_ops->ssthresh): the default is RFC 5681/6675's max(FlightSize/2, 2*SMSS)
    // (TcpAlgorithmBase::calculateSsthreshForFastRecovery), but CUBIC applies its own
    // beta (cwnd*0.7) -- hardcoding FlightSize/2 here gave CUBIC connections the wrong
    // post-recovery ssthresh (the fast_recovery/PRR scripts are all CUBIC). Capture the
    // pre-reduction cwnd first for PRR's priorCwnd (Linux tp->prior_cwnd = tp->snd_cwnd),
    // not the old snd_cwnd*2 which assumed a /2 factor.
    uint32_t priorCwnd = state->snd_cwnd;
    state->ssthresh = state->snd_cwnd = conn->getTcpAlgorithmForUpdate()->calculateSsthreshForFastRecovery();
    conn->emit(cwndSignal, state->snd_cwnd);
    conn->emit(ssthreshSignal, state->ssthresh);

    // RFC 6937: from here on the sending rate is paced by PRR rather than by the
    // reduced cwnd above; snapshot the pre-reduction cwnd and reset the counters.
    if (state->prrEnabled) {
        state->priorCwnd = priorCwnd;
        state->prrDelivered = 0;
        state->prrOut = 0;
        EV_INFO << "PRR fast recovery: entering, priorCwnd=" << state->priorCwnd
                << " ssthresh=" << state->ssthresh << "\n";
        // Run PRR on the entry ACK itself, exactly as Linux tcp_fastretrans_alert
        // calls tcp_cwnd_reduction() BEFORE tcp_xmit_retransmit_queue(). This
        // clamps snd_cwnd to pipe+sndcnt (~1 segment on entry) so the
        // retransmitOneSegment() + stepC() below send only sndcnt worth. Without
        // it snd_cwnd stays at the full reduced ssthresh and stepC's cwnd-pipe
        // loop floods every RACK-marked-lost segment at once -- a premature
        // multi-segment retransmit burst (Linux sends just the first hole and
        // paces the rest over later ACKs). step4() is only reached from the
        // duplicate-ACK path, so snd_una has not advanced (sndUnaAdvanced=false);
        // on the reo-timer entry there is no new delivery, prrNewlyDelivered()==0,
        // and prrCwndReduction() is an early-return no-op (behavior unchanged).
        prrCwndReduction((int)prrNewlyDelivered(), 0, false);
    }

    //"
    // (4.3) Retransmit the first data segment presumed dropped -- the
    //       segment starting with sequence number HighACK + 1.  To
    //       prevent repeated retransmission of the same data or a
    //       premature rescue retransmission, set both HighRxt and
    //       RescueRxt to the highest sequence number in the
    //       retransmitted segment.
    //"
    conn->retransmitOneSegment(false); // this also sends retransmitted segments

    //"
    // (4.4) Run SetPipe ()
    //       Set a "pipe" variable to the number of outstanding octets
    //       currently "in the pipe"; this is the data which has been
    //       sent by the TCP sender but for which no cumulative or
    //       selective acknowledgment has been received and the data has
    //       not been determined to have been dropped in the network.
    //       It is assumed that the data is still traversing the network
    //       path.
    //"
    setPipe();

    //"
    // (4.5) In order to take advantage of potential additional
    //       available cwnd, proceed to step (C) below.
    //"
    stepC();
}

void Rfc6675Recovery::receivedDuplicateAck()
{
    //"
    // If the incoming ACK is a duplicate acknowledgment per the definition
    // in Section 2 (regardless of its status as a cumulative
    // acknowledgment), and the TCP is not currently in loss recovery, the
    // TCP MUST increase DupAcks by one and take the following steps:
    //"
    if (!state->lossRecovery) {
        //"
        // (1) If DupAcks >= DupThresh, go to step (4).
        //     Note: This check covers the case when a TCP receives SACK
        //     information for multiple segments smaller than SMSS, which can
        //     potentially prevent IsLost() (next step) from declaring a segment
        //     as lost.
        //"
        if (state->dupacks >= state->dupthresh)
            step4();
        else {
            //"
            // (2) If DupAcks < DupThresh but IsLost (HighACK + 1) returns true --
            //     indicating at least three segments have arrived above the current
            //     cumulative acknowledgment point, which is taken to indicate loss
            //     -- go to step (4).
            //"
            if (isLost(state->snd_una + 1))
                step4();
            else {
                //"
                // (3) The TCP MAY transmit previously unsent data segments as per
                //     Limited Transmit [RFC5681], except that the number of octets
                //     which may be sent is governed by pipe and cwnd as follows:
                //"

                //"
                // (3.1) Set HighRxt to HighACK.
                //"
                state->highRxt = state->snd_una;

                //"
                // (3.2) Run SetPipe ().
                //"
                setPipe();

                //"
                // (3.3) If (cwnd - pipe) >= 1 SMSS, there exists previously unsent
                //       data, and the receiver's advertised window allows, transmit
                //       up to 1 SMSS of data starting with the octet HighData+1 and
                //       update HighData to reflect this transmission, then return
                //       to (3.2).
                //"
                while ((int32_t)state->snd_cwnd - (int32_t)state->pipe >= (int32_t)state->snd_mss) {
                    uint32_t seqNum;
                    if (!nextSeg(seqNum))
                        break;
                    // Limited Transmit (RFC 3042 / RFC 6675 step 3.3) transmits only
                    // PREVIOUSLY UNSENT data (HighData+1), never a retransmission. In RACK
                    // mode (lossDetectionMode==1) nextSeg()'s rule-3 "last resort" clause
                    // would otherwise return an old unSACKed segment (== snd_una on the first
                    // SACK) and retransmit the first hole a dupack early -- Linux only
                    // retransmits once RACK's reordering timer enters recovery. Restrict this
                    // pre-recovery path to new data there; classic recovery keeps its behavior.
                    if (state->lossDetectionMode == 1 && seqLess(seqNum, state->snd_max))
                        break;
                    if (seqLE(seqNum + state->snd_mss, state->snd_una + state->snd_wnd)) {
                        state->snd_nxt = seqNum;
                        uint32_t sentBytes = conn->sendSegment(state->snd_mss);
                        state->pipe += sentBytes;
                    }
                    else
                        break;
                }

                //"
                // (3.4) Terminate processing of this ACK.
                //"
            }
        }
    }
    else {
        // Already in loss recovery and this ACK is a (SACK-carrying) duplicate --
        // snd_una did not advance. RFC 6937 PRR must still run here so cwnd tracks the
        // bytes this ACK newly SACKed (Linux tcp_cwnd_reduction runs on EVERY ACK in
        // recovery); without it a pure-SACK recovery leaves cwnd frozen below pipe after
        // the entry retransmit and stalls into an RTO. sndUnaAdvanced=false.
        if (state->prrEnabled)
            prrCwndReduction((int)prrNewlyDelivered(), 0, false /* snd_una not advanced */);

        stepA();
        stepB();
        stepC();
    }
}


bool Rfc6675Recovery::processSACKOption(const Ptr<const TcpHeader>& tcpHeader, const TcpOptionSack& option)
{
    if (option.getLength() % 8 != 2) {
        EV_ERROR << "ERROR: option length incorrect\n";
        return false;
    }

    uint n = option.getSackItemArraySize();
    ASSERT(option.getLength() == 2 + n * 8);

    if (!state->sack_enabled) {
        EV_ERROR << "ERROR: " << n << " SACK(s) received, but sack_enabled is set to false\n";
        return false;
    }

    if (conn->getFsmState() != TCP_S_SYN_RCVD && conn->getFsmState() != TCP_S_ESTABLISHED
        && conn->getFsmState() != TCP_S_FIN_WAIT_1 && conn->getFsmState() != TCP_S_FIN_WAIT_2)
    {
        EV_ERROR << "ERROR: Tcp Header Option SACK received, but in unexpected state\n";
        return false;
    }

    if (n > 0) { // sacks present?
        EV_INFO << n << " SACK(s) received:\n";
        for (uint i = 0; i < n; i++) {
            Sack tmp;
            tmp.setStart(option.getSackItem(i).getStart());
            tmp.setEnd(option.getSackItem(i).getEnd());

            EV_INFO << (i + 1) << ". SACK: " << tmp.str() << endl;

            // check for D-SACK
            if (i == 0 && seqLE(tmp.getEnd(), tcpHeader->getAckNo())) {
                // RFC 2883, page 8:
                //"
                // In order for the sender to check that the first (D)SACK block of an
                // acknowledgement in fact acknowledges duplicate data, the sender
                // should compare the sequence space in the first SACK block to the
                // cumulative ACK which is carried IN THE SAME PACKET.  If the SACK
                // sequence space is less than this cumulative ACK, it is an indication
                // that the segment identified by the SACK block has been received more
                // than once by the receiver.  An implementation MUST NOT compare the
                // sequence space in the SACK block to the TCP state variable snd.una
                // (which carries the total cumulative ACK), as this may result in the
                // wrong conclusion if ACK packets are reordered.
                //"
                EV_DETAIL << "Received D-SACK below cumulative ACK=" << tcpHeader->getAckNo()
                          << " D-SACK: " << tmp.str() << endl;
                // RFC 2883: the segment identified by this block was received more
                // than once. Record it so the loss-undo logic can detect a spurious
                // retransmission (the RFC deliberately leaves the action unspecified).
                state->dsackSeen = true;
                state->dsackBytes = tmp.getEnd() - tmp.getStart();
                // a D-SACK also reveals reordering of the (spuriously retransmitted)
                // segment: grow the reordering degree so it stops recurring.
                if (state->adaptiveReorderingEnabled)
                    checkSackReordering(tmp.getStart());
                // Note: RFC 2883 does not specify what should be done in this case.
                // RFC 2883, page 9:
                //"
                // 5. Detection of Duplicate Packets
                // (...) This document does not specify what action a TCP implementation should
                // take in these cases. The extension to the SACK option simply enables
                // the sender to detect each of these cases.(...)
                //"
            }
            else if (i == 0 && n > 1 && seqGreater(tmp.getEnd(), tcpHeader->getAckNo())) {
                // RFC 2883, page 8:
                //"
                // If the sequence space in the first SACK block is greater than the
                // cumulative ACK, then the sender next compares the sequence space in
                // the first SACK block with the sequence space in the second SACK
                // block, if there is one.  This comparison can determine if the first
                // SACK block is reporting duplicate data that lies above the cumulative
                // ACK.
                //"
                Sack tmp2(option.getSackItem(1).getStart(), option.getSackItem(1).getEnd());

                if (tmp2.contains(tmp)) {
                    EV_DETAIL << "Received D-SACK above cumulative ACK=" << tcpHeader->getAckNo()
                              << " D-SACK: " << tmp.str()
                              << ", SACK: " << tmp2.str() << endl;
                    // RFC 2883: duplicate data above the cumulative ACK; record it
                    // for the loss-undo logic.
                    state->dsackSeen = true;
                    state->dsackBytes = tmp.getEnd() - tmp.getStart();
                    // a D-SACK also reveals reordering of the (spuriously retransmitted)
                    // segment: grow the reordering degree so it stops recurring.
                    if (state->adaptiveReorderingEnabled)
                        checkSackReordering(tmp.getStart());
                    // Note: RFC 2883 does not specify what should be done in this case.
                    // RFC 2883, page 9:
                    //"
                    // 5. Detection of Duplicate Packets
                    // (...) This document does not specify what action a TCP implementation should
                    // take in these cases. The extension to the SACK option simply enables
                    // the sender to detect each of these cases.(...)
                    //"
                }
            }

            if (seqGreater(tmp.getEnd(), tcpHeader->getAckNo()) && seqGreater(tmp.getEnd(), state->snd_una)) {
                // FACK before this block is applied: needed to recognize that the
                // block NEWLY sacks data below the highest already-SACKed sequence.
                uint32_t fackBefore = conn->getRexmitQueue()->getHighestSackedSeqNum();
                uint32_t newlySackedLow = conn->getRexmitQueueForUpdate()->setSackedBit(tmp.getStart(), tmp.getEnd());
                // Reordering detection (Linux tcp_sacktag_one/tcp_check_sack_reordering):
                // a never-retransmitted range newly SACKed BELOW the prior FACK proves
                // the network delivered it out of order -- data above it arrived first.
                // A re-reported or merely grown block returns newlySackedLow at/above
                // fackBefore and is ignored, as are SACKs of retransmissions.
                // F-RTO (SACK side): newly SACKed data that was never retransmitted
                // likewise proves the original flight arrived.
                if (state->frtoActive && newlySackedLow != 0)
                    state->frtoOrigAcked = true;
                if (state->adaptiveReorderingEnabled && newlySackedLow != 0
                        && fackBefore != 0 && seqLess(newlySackedLow, fackBefore))
                    checkSackReordering(newlySackedLow);
            }
            else
                EV_DETAIL << "Received SACK below total cumulative ACK snd_una=" << state->snd_una << "\n";
        }
        // Loss marking is exclusive per mode (Linux tcp_identify_packet_loss):
        // under RACK only time-based marking below may set lost -- the DupThresh
        // region rule would pre-mark burst holes on the first SACK, which both
        // defeats the reordering-window timer (already-lost regions are skipped
        // as candidates) and over-counts small (sub-MSS) SACKed regions against
        // a segment threshold.
        if (state->lossDetectionMode != 1)
            conn->getRexmitQueueForUpdate()->updateLost();

        state->rcv_sacks += n; // total counter, no current number

        conn->emit(rcvSacksSignal, state->rcv_sacks);

        // update scoreboard
        // TODO this is wrong because discardUpTo can delete sackedBytes,
        // and the SACK option can add the same amount of new sack bytes
        // leading to no changes in number of sacked bytes?
        state->sackedBytes_old = state->sackedBytes; // needed for RFC 3042 to check if last dupAck contained new sack information
        state->sackedBytes = conn->getRexmitQueue()->getTotalAmountOfSackedBytes();

        conn->emit(sackedBytesSignal, state->sackedBytes);

        // delivered-bytes accounting (RFC 8985/6937): count bytes newly SACKed by
        // this segment. Cumulatively-acked bytes are counted separately where
        // snd_una advances (process_ACK).
        if (state->sackedBytes > state->sackedBytes_old) {
            state->deliveredBytes += state->sackedBytes - state->sackedBytes_old;
            conn->emit(deliveredSignal, (unsigned long)state->deliveredBytes);
        }

        // RACK time-based loss detection runs on every ACK carrying new SACK info
        if (state->lossDetectionMode == 1)
            rackDetectAndMarkLost();
    }
    return true;
}

bool Rfc6675Recovery::isLost(uint32_t seqNum)
{
    ASSERT(state->sack_enabled);

    // RFC 6675, page 5:
    //"
    // This routine returns whether the given sequence number is
    // considered to be lost.  The routine returns true when either
    // DupThresh discontiguous SACKed sequences have arrived above
    // 'SeqNum' or more than (DupThresh - 1) * SMSS bytes with
    // numbers greater than 'SeqNum' have been SACKed.  Otherwise,
    // the routine returns false.
    //"
    ASSERT(seqGE(seqNum, state->snd_una)); // HighAck = snd_una - 1

    // RACK mode: a segment is lost iff RACK has marked its region lost (by time).
    // A seqNum not tracked by the rexmit queue (below its start, or at/above its
    // end) has no region and therefore cannot be marked lost -- guard getRegion,
    // whose precondition is begin <= seqNum < end. This can happen for a segment
    // whose range was already discarded, or before anything is queued (e.g. the
    // TFO fallback path that re-sends a SYN).
    if (state->lossDetectionMode == 1) {
        auto rexmitQueue = conn->getRexmitQueue();
        if (rexmitQueue->getQueueLength() == 0
                || seqLess(seqNum, rexmitQueue->getBufferStartSeq())
                || seqGE(seqNum, rexmitQueue->getBufferEndSeq()))
            return false;
        return rexmitQueue->getRegion(seqNum).lost;
    }

    // state->reordering equals state->dupthresh unless adaptive reordering has grown
    // it (static DupThresh otherwise), so this is inert by default.
    bool isLost = (conn->getRexmitQueue()->getNumOfDiscontiguousSacks(seqNum) >= state->reordering
                   || conn->getRexmitQueue()->getAmountOfSackedBytes(seqNum) > (state->reordering - 1) * state->snd_mss);

    return isLost;
}

uint32_t Rfc6675Recovery::rackDetectAndMarkLost(bool fromReoTimer)
{
    if (conn->getRexmitQueue() == nullptr || !state->sack_enabled)
        return 0;

    // (1) advance the RACK reference: the most recently *sent* segment among those
    // that have been delivered (SACKed). Skip retransmitted segments whose RTT is
    // below the connection minimum RTT (ambiguous, Karn-style).
    for (const auto& region : conn->getRexmitQueue()->rexmitQueue) {
        if (!region.sacked)
            continue;
        // Skip a sub-MSS SACKed TAIL fragment: Linux's tcp_match_skb_to_sack
        // fragments a partially-covered skb only at MSS boundaries, so a lone
        // byte-range SACK of a bigger skb's tail (fr-4pkt-fack-last-byte's
        // "sack 4000:4001") never gets tagged and never advances the kernel's
        // RACK reference -- TLP fires there instead of a RACK retransmit.
        // A WHOLE small skb (e.g. a fully-SACKed 400B MSG_EOR chunk) IS
        // tagged and DOES advance RACK (eor/no-coalesce-retrans pins that),
        // so only the buffer-tail fragment case is skipped. (Narrowed
        // reinstatement of a guard lost in the WS-3 recovery rewrite.)
        // A region that STARTS at a genuine transmission boundary is a whole
        // (small) segment, not a split-off fragment -- Linux tags it, so it
        // must advance the reference (client_accecn_options_lost pins the
        // SACKed 976-byte write tail arming RACK for its two full siblings).
        if (state->snd_mss > 0 && region.endSeqNum - region.beginSeqNum < state->snd_mss
            && region.endSeqNum == state->snd_max
            && !conn->getRexmitQueue()->isTransmissionStart(region.beginSeqNum))
            continue;
        simtime_t xmit = region.lastSentTime;
        simtime_t rtt = simTime() - xmit;
        if (region.transmitCount > 1 && state->minRtt > 0 && rtt < state->minRtt)
            continue;
        if (xmit > state->rackXmitTime
            || (xmit == state->rackXmitTime && seqGreater(region.endSeqNum, state->rackEndSeq)))
        {
            state->rackXmitTime = xmit;
            state->rackEndSeq = region.endSeqNum;
            state->rackRtt = rtt;
        }
    }

    if (state->rackXmitTime == 0)
        return 0;

    // (2) reordering window (Linux tcp_rack_reo_wnd): the default is a min_rtt/4
    // settling delay (capped at srtt/8) to tolerate mild reordering. Only when
    // reordering has NEVER been observed on the connection may RACK be aggressive
    // (reo_wnd = 0) -- and then only during recovery, or once DupThresh-worth of
    // segments are already SACKed (the classic dupthresh entry point). The
    // inverse rule (0 by default, min_rtt/4 after reordering) would let a single
    // SACK mark same-burst segments lost and enter recovery on the FIRST dupack.
    simtime_t reoWnd;
    // Linux's tcp_rack_reo_wnd input is tp->sacked_out, a PACKET count: divide
    // by the options-adjusted effective MSS, the size data segments are
    // actually cut to -- dividing by snd_mss undercounts (3 sacked 1000-byte
    // segments / mss 1012 = 2 < DupThresh) and misses the aggressive reo_wnd=0
    // clause, deferring recovery entry to the quantized reo timer where Linux
    // enters on the ACK itself (client-ack-dropped-then-recovery pins the
    // 2000-byte recovery-entry retransmit burst at +0).
    uint32_t segSize = state->snd_effmss > 0 ? state->snd_effmss : state->snd_mss;
    uint32_t sackedSegs = segSize > 0 ? state->sackedBytes / segSize : 0;
    if (!state->rackReordSeen && (state->lossRecovery || sackedSegs >= state->reordering))
        reoWnd = 0;
    else {
        // minRtt is only populated once a data RTT has been measured; on the very
        // first flight (dupacks arriving before any cumulative ACK) it is still 0.
        // Linux's min_rtt is seeded from the handshake, so it is never 0 by the
        // time SACKs arrive -- approximate that with this ACK's own RACK RTT.
        simtime_t minRtt = state->minRtt > 0 ? state->minRtt : state->rackRtt;
        reoWnd = minRtt / 4;
        if (state->srtt > 0 && state->srtt / 8 < reoWnd)
            reoWnd = state->srtt / 8;
    }

    // (3) mark as lost any earlier-sent, still-unacked segment for which at least
    // RACK.rtt + reo_wnd has elapsed since it was (last) sent. The comparison is
    // INCLUSIVE (Linux tcp_rack_detect_loss marks on remaining <= 0, i.e.
    // elapsed >= rtt + reo_wnd): with a whole flight transmitted in one burst --
    // the norm in a discrete-event simulation, where every segment of a window
    // carries the IDENTICAL send timestamp -- a lost head segment's elapsed time
    // always exactly EQUALS the RACK RTT derived from its SACKed burst-mates
    // (both measure simTime() - burstTime), so a strict > could never mark it,
    // no matter how much time passed, and recovery stalled into an RTO.
    std::vector<std::pair<uint32_t, uint32_t>> toMark;
    std::vector<std::pair<uint32_t, uint32_t>> toClearRexmit;
    simtime_t minRemaining = SIMTIME_MAX; // earliest not-yet-matured deadline
    for (const auto& region : conn->getRexmitQueue()->rexmitQueue) {
        if (region.sacked)
            continue;
        // A lost region whose RETRANSMISSION is still presumed in flight is a
        // candidate too: its lastSentTime is the retransmit time, and if that
        // matures against the reordering window (a SACK arrived for data sent
        // AFTER the retransmission), the retransmission itself was lost --
        // Linux tcp_mark_skb_lost then clears TCPCB_SACKED_RETRANS so the
        // range is sent once more (prr-ss-30pkt: the re-retransmit of
        // 1001:2001 when sack 2001:12001 proves the first rexmit died).
        // A lost region already awaiting (re)transmission needs nothing.
        if (region.lost && !region.rexmitted)
            continue;
        bool earlier = (region.lastSentTime < state->rackXmitTime)
            || (region.lastSentTime == state->rackXmitTime && seqLE(region.endSeqNum, state->rackEndSeq));
        if (!earlier)
            continue;
        simtime_t remaining = state->rackRtt + reoWnd - (simTime() - region.lastSentTime);
        if (remaining <= 0) {
            if (region.lost)
                toClearRexmit.push_back(std::make_pair(region.beginSeqNum, region.endSeqNum));
            else
                toMark.push_back(std::make_pair(region.beginSeqNum, region.endSeqNum));
        }
        else if (remaining < minRemaining)
            minRemaining = remaining;
    }

    uint32_t lostBytes = 0;
    for (auto& r : toMark) {
        conn->getRexmitQueueForUpdate()->markLost(r.first, r.second);
        lostBytes += r.second - r.first;
    }
    for (auto& r : toClearRexmit) {
        conn->getRexmitQueueForUpdate()->clearRexmitted(r.first, r.second);
        lostBytes += r.second - r.first;
        EV_INFO << "RACK: retransmission of [" << r.first << ", " << r.second << ") presumed lost, will re-send\n";
    }
    if (lostBytes > 0)
        EV_INFO << "RACK: marked " << lostBytes << " bytes lost by time (RACK.rtt=" << state->rackRtt << ")\n";

    // Arm the RACK reordering timer for the earliest deadline that has not matured
    // yet (Linux ICSK_TIME_REO_TIMEOUT): dupacks stop arriving once the receiver has
    // ACKed everything it got, so without this timer a deadline maturing between ACKs
    // -- e.g. on a tail flight -- would only ever be noticed by the much later RTO.
    // When the ACK path itself marks segments lost, arm at ZERO delay instead: the
    // marking happens during SACK processing, BEFORE the cumulative ACK advances
    // snd_una, so recovery entry must be deferred past the current event (Linux runs
    // tcp_fastretrans_alert after tcp_clean_rtx_queue for the same reason). The
    // timer handler re-runs detection and acts on the standing lost marks. From the
    // timer handler itself the caller acts directly, so only the not-yet-matured
    // deadline (if any) is re-armed there.
    simtime_t armDelay = minRemaining != SIMTIME_MAX ? minRemaining : simtime_t(-1);
    if (lostBytes > 0 && !fromReoTimer)
        armDelay = SIMTIME_ZERO;
    conn->rescheduleRackReoTimer(armDelay);

    return lostBytes;
}

void Rfc6675Recovery::undoInit()
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

bool Rfc6675Recovery::packetDelayed() const
{
    // Eifel (RFC 3522 / Linux tcp_packet_delayed): the most recent ACK echoed a
    // timestamp OLDER than our first retransmission's send time, so the receiver
    // generated it from the ORIGINAL transmission -- the retransmission (and the
    // congestion response that came with it) was spurious.
    return state->ts_enabled && state->retransStampTS != 0
        && state->lastRcvdTSecr != 0
        && seqLess(state->lastRcvdTSecr, state->retransStampTS);
}

bool Rfc6675Recovery::mayUndo() const
{
    // Linux tcp_may_undo(): undo when every retransmission of the episode has been
    // D-SACKed (undoRetrans == 0), or when the Eifel timestamp test proves the
    // retransmission was answered from the original transmission.
    return state->undoMarker != 0 && (state->undoRetrans == 0 || packetDelayed());
}

void Rfc6675Recovery::undoCwndReduction()
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

uint32_t Rfc6675Recovery::prrNewlyDelivered() const
{
    // bytes newly cumulatively-acked + selectively-acked by the ACK being processed
    // (snapshot taken at the top of process_RCV_SEGMENT)
    return (uint32_t)(state->deliveredBytes - state->prrDeliveredMark);
}

void Rfc6675Recovery::prrCwndReduction(int newlyAckedSacked, int newlyLost, bool sndUnaAdvanced)
{
    // RFC 6937 / Linux tcp_cwnd_reduction(): proportional rate reduction. All
    // quantities are in bytes (Linux counts packets); 1 packet == snd_mss bytes.
    if (newlyAckedSacked <= 0 || state->priorCwnd == 0)
        return;

    setPipe();
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

void Rfc6675Recovery::prrEndCwndReduction()
{
    // RFC 6937 / Linux tcp_end_cwnd_reduction(): set cwnd to ssthresh on leaving recovery.
    state->snd_cwnd = state->ssthresh;
    conn->emit(cwndSignal, state->snd_cwnd);
    EV_INFO << "PRR: leaving fast recovery, cwnd=ssthresh=" << state->snd_cwnd << "\n";
}

void Rfc6675Recovery::checkSackReordering(uint32_t lowSeq)
{
    // Linux tcp_check_sack_reordering(): reordering is proven when data at lowSeq
    // was delivered while a higher sequence number (fack) had already been SACKed.
    auto rexmitQueue = conn->getRexmitQueue();
    if (rexmitQueue == nullptr || !state->sack_enabled)
        return;
    uint32_t fack = rexmitQueue->getHighestSackedSeqNum();
    if (fack == 0 || seqGE(lowSeq, fack))
        return;
    uint32_t metric = fack - lowSeq;
    if (state->snd_mss != 0 && metric > state->reordering * state->snd_mss) {
        uint32_t newReordering = (metric + state->snd_mss - 1) / state->snd_mss;
        state->reordering = std::min(newReordering, state->maxReordering);
        EV_DETAIL << "reordering degree updated to " << state->reordering << "\n";
    }
    state->rackReordSeen = true; // activate RACK's reordering window as well
}

void Rfc6675Recovery::onRexmitTimeout()
{
    // F-RTO (RFC 5682, SACK-enhanced): open a spurious-RTO detection episode.
    // Capture the undo context BEFORE the RTO's ssthresh/cwnd reduction (sec 3.2:
    // recurring timeouts on the same SND.UNA keep the ORIGINAL context, hence the
    // undoMarker guard), and remember snd_max ("recover") so the episode can be
    // closed once everything outstanding at the RTO has been accounted for.
    if (state->frtoEnabled && state->sack_enabled) {
        if (state->undoMarker == 0)
            undoInit();
        state->frtoActive = true;
        state->frtoHighSeq = state->snd_max;
        state->frtoOrigAcked = false;
    }
    else if (state->lossUndoEnabled && state->undoMarker == 0) {
        undoInit();
    }
}

void Rfc6675Recovery::processFrtoEpisode()
{
    if (!state->frtoActive)
        return;
    if (state->frtoOrigAcked) {
        // RFC 5682 step 3.b: never-retransmitted data was (s)acked -- the original
        // flight arrived, so the RTO was spurious. Restore the pre-RTO cwnd/ssthresh
        // (Linux tcp_try_undo_loss(frto_undo=true)) and forget the loss marks:
        // nothing was actually lost.
        EV_INFO << "F-RTO: spurious retransmission timeout detected, undoing the RTO response\n";
        undoCwndReduction();
        conn->getRexmitQueueForUpdate()->resetLostBit();
        state->afterRto = false;
        state->rexmit_count = 0; // Linux clears icsk_retransmits on the undo
        state->frtoActive = false;
        state->frtoOrigAcked = false;
    }
    else if (seqGE(state->snd_una, state->frtoHighSeq)) {
        // everything outstanding at the RTO has been accounted for through the
        // conventional recovery: the loss was real, close the episode.
        state->frtoActive = false;
        state->undoMarker = 0;
    }
}

void Rfc6675Recovery::reoTimeout()
{
    // RACK marked further bytes lost while no ACK was arriving. If we are not yet
    // recovering, this is the fast-retransmit trigger RACK exists to provide;
    // otherwise just push out whatever the scoreboard now says is missing.
    if (!state->lossRecovery)
        step4();
    else
        stepC();
}

void Rfc6675Recovery::segmentsAcked(uint32_t fromSeq, uint32_t toSeq)
{
    // F-RTO (RFC 5682 sec 3.1 step 3.b, cumulative side): the scoreboard for
    // [fromSeq, toSeq) is still intact here. If any part of the newly
    // cumulatively-acked range was transmitted exactly once -- i.e. is NOT one of
    // the post-RTO retransmissions -- then the original flight (or part of it)
    // reached the receiver, so the timeout was spurious.
    if (state->frtoActive && state->sack_enabled) {
        auto frq = conn->getRexmitQueue();
        if (frq != nullptr && frq->getQueueLength() > 0) {
            for (uint32_t seq = std::max(fromSeq, frq->getBufferStartSeq());
                 seqLess(seq, std::min(toSeq, frq->getBufferEndSeq())); )
            {
                const auto& region = frq->getRegion(seq);
                if (region.transmitCount <= 1) {
                    state->frtoOrigAcked = true;
                    break;
                }
                seq = region.endSeqNum;
            }
        }
    }
    processFrtoEpisode();

    // Adaptive reordering: if this cumulatively-acked segment was never retransmitted
    // yet sits below already-SACKed data, it was merely reordered (not lost) -- grow the
    // learned reordering degree so it stops causing spurious fast retransmits.
    if (state->adaptiveReorderingEnabled && state->sack_enabled) {
        auto rq = conn->getRexmitQueue();
        if (rq != nullptr && rq->getQueueLength() > 0
            && seqLE(rq->getBufferStartSeq(), fromSeq) && seqLess(fromSeq, rq->getBufferEndSeq())
            && rq->getRegion(fromSeq).transmitCount <= 1)
        {
            checkSackReordering(fromSeq);
        }
    }

    if (!state->lossUndoEnabled)
        return;

    // RFC 2883 loss undo. This runs on every ACK that advances snd_una, in or out of
    // loss recovery -- deliberately not only while recovering, because the D-SACK that
    // proves a retransmission spurious usually arrives only after the delayed original
    // has been delivered, by which time the recovery episode has already ended. Linux
    // likewise checks undo from the ACK path independently of the congestion state.
    if (state->dsackSeen && state->undoMarker != 0 && state->undoRetrans > 0) {
        uint32_t segs = (state->dsackBytes + state->snd_mss - 1) / state->snd_mss;
        state->undoRetrans -= (int32_t)segs;
        if (state->undoRetrans < 0)
            state->undoRetrans = 0;
    }

    if (mayUndo()) {
        // every retransmission of this episode was D-SACKed: the reduction was
        // needless, so restore cwnd/ssthresh (and leave recovery if still in it).
        undoCwndReduction();
        state->lossRecovery = false;
    }
}

void Rfc6675Recovery::dataSent(uint32_t fromSeq)
{
    // RFC 6937 accounting: bytes transmitted during the current recovery episode.
    if (state->prrEnabled && state->lossRecovery && seqGreater(state->snd_nxt, fromSeq))
        state->prrOut += state->snd_nxt - fromSeq;
}

void Rfc6675Recovery::segmentRetransmitted(uint32_t fromSeq, uint32_t toSeq)
{
    if (state->prrEnabled && state->lossRecovery && seqGreater(toSeq, fromSeq))
        state->prrOut += toSeq - fromSeq;

    // Eifel (RFC 3522 / Linux retrans_stamp): stamp the FIRST retransmission of the
    // episode with our TS clock. An ACK later echoing a TSecr OLDER than this was
    // generated by the ORIGINAL transmission, proving the retransmission spurious.
    if (state->ts_enabled && state->retransStampTS == 0)
        state->retransStampTS = TcpConnection::convertSimtimeToTS(simTime());

    // Loss undo: count the retransmissions of this episode that still have to be
    // proven spurious (Linux increments undo_retrans per retransmitted skb).
    if (state->lossUndoEnabled && state->undoMarker != 0) {
        if (state->undoRetrans < 0)
            state->undoRetrans = 0;
        uint32_t segs = seqGreater(toSeq, fromSeq)
            ? (toSeq - fromSeq + state->snd_mss - 1) / state->snd_mss : 1;
        state->undoRetrans += (int32_t)segs;
    }
}

void Rfc6675Recovery::setPipe()
{
    ASSERT(state->sack_enabled);

    // RFC 6675, page 3:
    //"
    // "HighACK" is the sequence number of the highest byte of data that
    // has been cumulatively ACKed at a given point.
    //
    // "HighData" is the highest sequence number transmitted at a given
    // point.
    //
    // "HighRxt" is the highest sequence number which has been
    // retransmitted during the current loss recovery phase.
    //
    // "RescueRxt" is the highest sequence number which has been
    // optimistically retransmitted to prevent stalling of the ACK clock
    // when there is loss at the end of the window and no new data is
    // available for transmission.
    //
    // "Pipe" is a sender's estimate of the number of bytes outstanding
    // in the network.  This is used during recovery for limiting the
    // sender's sending rate.  The pipe variable allows TCP to use a
    // fundamentally different congestion control than specified in
    // [RFC5681].  The algorithm is often referred to as the "pipe
    // algorithm".
    //
    // "DupAcks" is the number of duplicate acknowledgments received
    // since the last cumulative acknowledgment.
    //"
    // HighAck = snd_una
    // HighData = snd_max

    state->highRxt = conn->getRexmitQueue()->getHighestRexmittedSeqNum();
    state->pipe = 0;
    uint32_t length = 0; // required for rexmitQueue->checkSackBlock()
    bool sacked; // required for rexmitQueue->checkSackBlock()
    bool rexmitted; // required for rexmitQueue->checkSackBlock()

    // RFC 6675, page 5:
    //"
    // This routine traverses the sequence space from HighACK to HighData
    // and MUST set the "pipe" variable to an estimate of the number of
    // octets that are currently in transit between the TCP sender and
    // the TCP receiver.  After initializing pipe to zero the following
    // steps are taken for each octet 'S1' in the sequence space between
    // HighACK and HighData that has not been SACKed:
    //"
    // HighData (snd_max) can sit one sequence number past the last DATA octet
    // when a FIN is outstanding: the FIN consumes a sequence number but is not
    // stored in the (data-only) rexmit queue. Scan only the sequence space the
    // scoreboard actually covers, or checkSackBlock() walks off its end and
    // aborts (seen on a TFO fallback that closes with data still in flight).
    uint32_t scanEnd = conn->getRexmitQueue()->getBufferEndSeq();
    if (seqLess(state->snd_max, scanEnd))
        scanEnd = state->snd_max;
    for (uint32_t s1 = state->snd_una; seqLess(s1, scanEnd); s1 += length) {
        conn->getRexmitQueue()->checkSackBlock(s1, length, sacked, rexmitted);

        if (!sacked) {
            // RFC 6675, page 5:
            //"
            // (a) If IsLost (S1) returns false:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     packets that have not been SACKed and have not been determined
            //     to have been lost (i.e., those segments that are still assumed
            //     to be in the network).
            //"
            if (isLost(s1) == false)
                state->pipe += length;

            // RFC 6675, pages 5:
            //"
            // (b) If S1 <= HighRxt:
            //
            //     Pipe is incremented by 1 octet.
            //
            //     The effect of this condition is that pipe is incremented for
            //     the retransmission of the octet.
            //
            //  Note that octets retransmitted without being considered lost are
            //  counted twice by the above mechanism.
            //"
            if (seqLess(s1, state->highRxt))
                state->pipe += length;
        }
    }

    conn->emit(pipeSignal, state->pipe);
}

bool Rfc6675Recovery::nextSeg(uint32_t& seqNum)
{
    ASSERT(state->sack_enabled);

    // RFC 6675, page 6:
    //"
    // This routine uses the scoreboard data structure maintained by the
    // Update() function to determine what to transmit based on the SACK
    // information that has arrived from the data receiver (and hence
    // been marked in the scoreboard).  NextSeg () MUST return the
    // sequence number range of the next segment that is to be
    // transmitted, per the following rules:
    //"

    state->highRxt = conn->getRexmitQueue()->getHighestRexmittedSeqNum();
    uint32_t highestSackedSeqNum = conn->getRexmitQueue()->getHighestSackedSeqNum();
    uint32_t shift = state->snd_mss;
    bool sacked = false; // required for rexmitQueue->checkSackBlock()
    bool rexmitted = false; // required for rexmitQueue->checkSackBlock()

    seqNum = 0;

    if (state->ts_enabled)
        shift -= B(TCP_OPTION_TS_SIZE).get();

    // RFC 6675, page 6:
    //"
    // (1) If there exists a smallest unSACKed sequence number 'S2' that
    // meets the following three criteria for determining loss, the
    // sequence range of one segment of up to SMSS octets starting
    // with S2 MUST be returned.
    //
    // (1.a) S2 is greater than HighRxt.
    //
    // (1.b) S2 is less than the highest octet covered by any
    //       received SACK.
    //
    // (1.c) IsLost (S2) returns true.
    //"

    // RACK mode: Linux tcp_xmit_retransmit_queue walks the whole rtx queue by
    // sequence with no HighRxt floor -- it skips SACKED_RETRANS entries and
    // (re)transmits anything marked LOST. That reaches a lost region BELOW the
    // highest retransmission whose rexmitted flag RACK just cleared (its first
    // retransmit died; prr-ss-30pkt pins the second '. 1001:2001' retransmit).
    // Rule (1.a)'s "S2 greater than HighRxt" would hide it forever.
    if (state->lossDetectionMode == 1) {
        for (const auto& region : conn->getRexmitQueue()->rexmitQueue) {
            if (!seqLess(region.beginSeqNum, highestSackedSeqNum))
                break;
            if (!region.sacked && region.lost && !region.rexmitted) {
                seqNum = region.beginSeqNum;
                return true;
            }
        }
    }
    else
    // Note: state->highRxt == RFC.HighRxt + 1
    for (uint32_t s2 = state->highRxt;
         seqLess(s2, state->snd_max) && seqLess(s2, highestSackedSeqNum);
         s2 += shift)
    {
        conn->getRexmitQueue()->checkSackBlock(s2, shift, sacked, rexmitted);

        if (!sacked) {
            if (isLost(s2)) { // 1.a and 1.b are true, see above "for" statement
                seqNum = s2;

                return true;
            }

            break; // !isLost(x) --> !isLost(x + d)
        }
    }

    // RFC 6675, page 6
    //"
    // (2) If no sequence number 'S2' per rule (1) exists but there
    // exists available unsent data and the receiver's advertised
    // window allows, the sequence range of one segment of up to SMSS
    // octets of previously unsent data starting with sequence number
    // HighData+1 MUST be returned.
    //"
    {
        // check how many unsent bytes we have
        uint32_t buffered = conn->getSendQueue()->getBytesAvailable(state->snd_max);
        uint32_t maxWindow = state->snd_wnd;
        // effectiveWindow: number of bytes we're allowed to send now. pipe may exceed
        // the advertised window (RFC 6675 setPipe counts retransmitted-not-lost octets
        // twice, and snd_wnd can shrink), so the difference must not wrap.
        uint32_t effectiveWin = maxWindow > state->pipe ? maxWindow - state->pipe : 0;

        if (buffered > 0 && effectiveWin >= state->snd_mss) {
            seqNum = state->snd_max; // HighData = snd_max

            return true;
        }
    }

    // RFC 6675, pages 6 and 7
    //"
    // (3) If the conditions for rules (1) and (2) fail, but there exists
    // an unSACKed sequence number 'S3' that meets the criteria for
    // detecting loss given in steps (1.a) and (1.b) above
    // (specifically excluding step (1.c)) then one segment of up to
    // SMSS octets starting with S3 MAY be returned.
    //
    // (4) If the conditions for (1), (2), and (3) fail, but there exists
    // outstanding unSACKed data, we provide the opportunity for a
    // single "rescue" retransmission per entry into loss recovery.
    // If HighACK is greater than RescueRxt (or RescueRxt is
    // undefined), then one segment of up to SMSS octets that MUST
    // include the highest outstanding unSACKed sequence number
    // SHOULD be returned, and RescueRxt set to RecoveryPoint.
    // HighRxt MUST NOT be updated.
    //
    // Note that rule (3) and (4) are a sort of retransmission "last resort".
    // They allow for retransmission of sequence numbers even when the
    // sender has less certainty a segment has been lost than as with
    // rule (1).  Retransmitting segments via rule (3) and (4) will help
    // sustain TCP's ACK clock and therefore can potentially help
    // avoid retransmission timeouts.  However, in sending these
    // segments, the sender has two copies of the same data considered
    // to be in the network (and also in the Pipe estimate, in the case of (3)).  When an
    // ACK or SACK arrives covering this retransmitted segment, the
    // sender cannot be sure exactly how much data left the network
    // (one of the two transmissions of the packet or both
    // transmissions of the packet).  Therefore the sender may
    // underestimate Pipe by considering both segments to have left
    // the network when it is possible that only one of the two has.
    //"
    // TODO: rule 4 clause
    {
        for (uint32_t s3 = state->highRxt;
             seqLess(s3, state->snd_max) && seqLess(s3, highestSackedSeqNum);
             s3 += shift)
        {
            conn->getRexmitQueue()->checkSackBlock(s3, shift, sacked, rexmitted);

            if (!sacked) {
                // 1.a and 1.b are true, see above "for" statement
                seqNum = s3;

                return true;
            }
        }
    }

    // RFC 6675, page 7:
    //"
    // (5) If the conditions for each of (1), (2), (3), and (4) are not
    // met, then NextSeg () MUST indicate failure, and no segment is
    // returned.
    //"
    seqNum = 0;

    return false;
}

void Rfc6675Recovery::sendDataDuringLossRecoveryPhase(uint32_t congestionWindow)
{
    ASSERT(state->sack_enabled && state->lossRecovery);

    // RFC 6675, page 9
    //"
    // (4.5) In order to take advantage of potential additional available
    // cwnd, proceed to step (C) below.
    // (...)
    // (C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
    // segments as follows:
    // (...)
    // (C.5) If cwnd - pipe >= 1 SMSS, return to (C.1)
    //"
    while (((int)congestionWindow - (int)state->pipe) >= (int)state->snd_mss) { // Note: Typecast needed to avoid prohibited transmissions
        // RFC 6675, page 9:
        //"
        // (C.1) The scoreboard MUST be queried via NextSeg () for the
        // sequence number range of the next segment to transmit (if any),
        // and the given segment sent.  If NextSeg () returns failure (no
        // data to send) return without sending anything (i.e., terminate
        // steps C.1 -- C.5).
        //"

        uint32_t seqNum;

        if (!nextSeg(seqNum)) // if nextSeg() returns false (=failure): terminate steps C.1 -- C.5
            break;

        uint32_t sentBytes = sendSegmentDuringLossRecoveryPhase(seqNum);
        // RFC 6675, page 9:
        //"
        // (C.4) The estimate of the amount of data outstanding in the
        // network must be updated by incrementing pipe by the number of
        // octets transmitted in (C.1).
        //"
        state->pipe += sentBytes;
    }
}

uint32_t Rfc6675Recovery::sendSegmentDuringLossRecoveryPhase(uint32_t seqNum)
{
    ASSERT(state->sack_enabled && state->lossRecovery);

    // start sending from seqNum
    state->snd_nxt = seqNum;

    uint32_t old_highRxt = conn->getRexmitQueue()->getHighestRexmittedSeqNum();

    // no need to check cwnd and rwnd - has already be done before
    // no need to check nagle - sending mss bytes
    uint32_t sentBytes = conn->sendSegment(state->snd_mss);

    uint32_t sentSeqNum = seqNum + sentBytes;

    if (state->send_fin && sentSeqNum == state->snd_fin_seq)
        sentSeqNum = sentSeqNum + 1;

    ASSERT(seqLE(state->snd_nxt, sentSeqNum));

    // RFC 6675, page 9:
    //"
    // (C.2) If any of the data octets sent in (C.1) are below HighData,
    // HighRxt MUST be set to the highest sequence number of the
    // retransmitted segment unless NextSeg () rule (4) was
    // invoked for this retransmission.
    //"
    // TODO: rule 4 clause
    if (seqLess(seqNum, state->snd_max)) { // HighData = snd_max
        state->highRxt = conn->getRexmitQueue()->getHighestRexmittedSeqNum();
    }

    // RFC 6675, page 9:
    //"
    // (C.3) If any of the data octets sent in (C.1) are above HighData,
    // HighData must be updated to reflect the transmission of
    // previously unsent data.
    //"
    if (seqGreater(sentSeqNum, state->snd_max)) { // HighData = snd_max
        state->snd_max = sentSeqNum;
        conn->emit(sndMaxSignal, state->snd_max);
    }

    conn->emit(unackedSignal, state->snd_max - state->snd_una);

    // RFC 6675, page 11:
    //"
    // 6   Managing the RTO Timer
    //
    // The standard TCP RTO estimator is defined in [RFC6288].  Due to the
    // fact that the SACK algorithm in this document can have an impact on
    // the behavior of the estimator, implementers may wish to consider how
    // the timer is managed.  [RFC6288] calls for the RTO timer to be
    // re-armed each time an ACK arrives that advances the cumulative ACK
    // point.  Because the algorithm presented in this document can keep the
    // ACK clock going through a fairly significant loss event,
    // (comparatively longer than the algorithm described in [RFC5681]), on
    // some networks the loss event could last longer than the RTO.  In this
    // case the RTO timer would expire prematurely and a segment that need
    // not be retransmitted would be resent.
    //
    // Therefore we give implementers the latitude to use the standard
    // [RFC6288] style RTO management or, optionally, a more careful variant
    // that re-arms the RTO timer on each retransmission that is sent during
    // recovery MAY be used.  This provides a more conservative timer than
    // specified in [RFC6288], and so may not always be an attractive
    // alternative.  However, in some cases it may prevent needless
    // retransmissions, go-back-N transmission and further reduction of the
    // congestion window.
    //"
    conn->getTcpAlgorithmForUpdate()->ackSent();

    if (old_highRxt != state->highRxt) {
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 5681, however optional in RFC 6675 if sent during recovery.
        EV_INFO << "Retransmission sent during recovery, restarting REXMIT timer.\n";
        conn->getTcpAlgorithmForUpdate()->restartRexmitTimer();
    }
    else // don't measure RTT for retransmitted packets
        conn->getTcpAlgorithmForUpdate()->dataSent(seqNum); // seqNum = old_snd_nxt

    return sentBytes;
}

TcpHeader Rfc6675Recovery::addSacks(const Ptr<TcpHeader>& tcpHeader)
{
    B options_len = B(0);
    B used_options_len = tcpHeader->getHeaderOptionArrayLength();
    bool dsack_inserted = false; // set if dsack is subsets of a bigger sack block recently reported

    uint32_t start = state->start_seqno;
    uint32_t end = state->end_seqno;

    // delete old sacks (below rcv_nxt), delete duplicates and print previous status of sacks_array:
    auto it = state->sacks_array.begin();
    EV_INFO << "Previous status of sacks_array: \n" << ((it != state->sacks_array.end()) ? "" : "\t EMPTY\n");

    while (it != state->sacks_array.end()) {
        if (seqLE(it->getEnd(), state->rcv_nxt) || it->empty()) {
            EV_DETAIL << "\t SACK in sacks_array: " << " " << it->str() << " delete now\n";
            it = state->sacks_array.erase(it);
        }
        else {
            EV_DETAIL << "\t SACK in sacks_array: " << " " << it->str() << endl;

            ASSERT(seqGE(it->getStart(), state->rcv_nxt));

            it++;
        }
    }

    if (used_options_len > TCP_OPTIONS_MAX_SIZE - TCP_OPTION_SACK_MIN_SIZE) {
        EV_ERROR << "ERROR: Failed to addSacks - at least 10 free bytes needed for SACK - used_options_len=" << used_options_len << endl;

        // reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;
        return *tcpHeader;
    }

    if (start != end) {
        if (state->dsack_enabled && state->snd_dsack) { // SequenceNo < rcv_nxt
            // RFC 2883, page 3:
            //"
            // (3) The left edge of the D-SACK block specifies the first sequence
            // number of the duplicate contiguous sequence, and the right edge of
            // the D-SACK block specifies the sequence number immediately following
            // the last sequence in the duplicate contiguous sequence.
            //"
            if (seqLess(start, state->rcv_nxt) && seqLess(state->rcv_nxt, end))
                end = state->rcv_nxt;

            dsack_inserted = true;
            Sack nSack(start, end);
            state->sacks_array.push_front(nSack);
            EV_DETAIL << "inserted DSACK entry: " << nSack.str() << "\n";
        }
        else if (seqGreater(end, state->rcv_nxt)) {
            uint32_t contStart = conn->getReceiveQueue()->getLE(start);
            uint32_t contEnd = conn->getReceiveQueue()->getRE(end);

            Sack newSack(contStart, contEnd);
            state->sacks_array.push_front(newSack);
            EV_DETAIL << "Inserted SACK entry: " << newSack.str() << "\n";
        }

        // RFC 2883, page 3:
        //"
        // (3) The left edge of the D-SACK block specifies the first sequence
        // number of the duplicate contiguous sequence, and the right edge of
        // the D-SACK block specifies the sequence number immediately following
        // the last sequence in the duplicate contiguous sequence."

        // RFC 2018, page 4:
        // "* The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,
        // unless that segment advanced the Acknowledgment Number field in
        // the header.  This assures that the ACK with the SACK option
        // reflects the most recent change in the data receiver's buffer
        // queue.
        //"

        // RFC 2018, page 4:
        //"
        // * The first SACK block (i.e., the one immediately following the
        // kind and length fields in the option) MUST specify the contiguous
        // block of data containing the segment which triggered this ACK,
        //"

        // RFC 2883, page 3:
        // (4) If the D-SACK block reports a duplicate contiguous sequence from
        // a (possibly larger) block of data in the receiver's data queue above
        // the cumulative acknowledgement, then the second SACK block in that
        // SACK option should specify that (possibly larger) block of data.
        //
        // (5) Following the SACK blocks described above for reporting duplicate
        // segments, additional SACK blocks can be used for reporting additional
        // blocks of data, as specified in RFC 2018.
        //"

        // RFC 2018, page 4:
        // * The SACK option SHOULD be filled out by repeating the most
        // recently reported SACK blocks (based on first SACK blocks in
        // previous SACK options) that are not subsets of a SACK block
        // already included in the SACK option being constructed.
        //"

        it = state->sacks_array.begin();
        if (dsack_inserted)
            it++;

        for (; it != state->sacks_array.end(); it++) {
            ASSERT(!it->empty());

            auto it2 = it;
            it2++;
            while (it2 != state->sacks_array.end()) {
                if (it->contains(*it2)) {
                    EV_DETAIL << "sack matched, delete contained : a=" << it->str() << ", b=" << it2->str() << endl;
                    it2 = state->sacks_array.erase(it2);
                }
                else
                    it2++;
            }
        }
    }

    uint n = state->sacks_array.size();

    uint maxnode = ((B(TCP_OPTIONS_MAX_SIZE - used_options_len).get()) - 2) / 8; // 2: option header, 8: size of one sack entry

    // Linux tcp_options_fit_accecn's SACK-reduction arm: when an AccECN option
    // is REQUIRED (a counter changed since the last one, accEcnOptMinFields>0),
    // give up SACK blocks -- but never below 2 -- so the option fits at the
    // kernel's canonical padding (nop,nop,TS = 12B; nop,nop,SACK = 4+8n). If
    // even 2 blocks plus the required fields don't fit, keep all blocks and
    // the option is omitted instead (sack_space_grab pins both directions:
    // the CE reply drops to 2 blocks + 2 fields, the ECT0 reply keeps 3
    // blocks and no option).
    if (state->accEcnNegotiated && state->accEcnOptionEnabled && state->sawAccEcnOpt
        && state->accEcnOptMinFields > 0 && n > 2)
    {
        uint32_t canonical = state->ts_enabled ? 12 : 0;
        uint32_t optAlign = ((2 + 3 * (uint32_t)state->accEcnOptMinFields) + 3) & ~3u;
        uint32_t maxBlocks = n;
        while (maxBlocks > 2 && canonical + (4 + 8 * maxBlocks) + optAlign > 40)
            maxBlocks--;
        if (canonical + (4 + 8 * maxBlocks) + optAlign <= 40 && maxBlocks < n)
            n = maxBlocks;
    }

    if (n > maxnode)
        n = maxnode;

    if (n == 0) {
        if (dsack_inserted)
            state->sacks_array.pop_front(); // delete DSACK entry

        // reset flags:
        state->snd_sack = false;
        state->snd_dsack = false;
        state->start_seqno = 0;
        state->end_seqno = 0;

        return *tcpHeader;
    }

    while (B(used_options_len).get() % 4 != 2)
        used_options_len++;

    ASSERT(B(used_options_len).get() % 4 == 2);

    TcpOptionSack *option = new TcpOptionSack();
    option->setLength(8 * n + 2);
    option->setSackItemArraySize(n);

    // write sacks from sacks_array to options
    uint counter = 0;

    for (it = state->sacks_array.begin(); it != state->sacks_array.end() && counter < n; it++) {
        ASSERT(it->getStart() != it->getEnd());
        option->setSackItem(counter++, *it);
    }

    // independent of "n" we always need 2 padding bytes (NOP) to make: (used_options_len % 4 == 0)
    options_len = used_options_len + TCP_OPTION_SACK_ENTRY_SIZE * n + TCP_OPTION_HEAD_SIZE; // 8 bytes for each SACK (n) + 2 bytes for kind&length

    ASSERT(options_len <= TCP_OPTIONS_MAX_SIZE); // Options length allowed? - maximum: 40 Bytes

    tcpHeader->appendHeaderOption(option);
    tcpHeader->setHeaderLength(TCP_MIN_HEADER_LENGTH + tcpHeader->getHeaderOptionArrayLength());
    tcpHeader->setChunkLength(tcpHeader->getHeaderLength());
    // update number of sent sacks
    state->snd_sacks += n;

    conn->emit(sndSacksSignal, state->snd_sacks);

    EV_INFO << n << " SACK(s) added to header:\n";

    for (uint t = 0; t < n; t++) {
        EV_INFO << t << ". SACK:" << " [" << option->getSackItem(t).getStart() << ".." << option->getSackItem(t).getEnd() << ")";

        if (t == 0) {
            if (state->snd_dsack)
                EV_INFO << " (D-SACK)";
            else if (seqLE(option->getSackItem(t).getEnd(), state->rcv_nxt)) {
                EV_INFO << " (received segment filled out a gap)";
                state->snd_dsack = true; // Note: Set snd_dsack to delete first sack from sacks_array
            }
        }

        EV_INFO << endl;
    }

    // RFC 2883, page 3:
    //"
    // (1) A D-SACK block is only used to report a duplicate contiguous
    // sequence of data received by the receiver in the most recent packet.
    //
    // (2) Each duplicate contiguous sequence of data received is reported
    // in at most one D-SACK block.  (I.e., the receiver sends two identical
    // D-SACK blocks in subsequent packets only if the receiver receives two
    // duplicate segments.)
    //
    // In case of d-sack: delete first sack (d-sack) and move old sacks by one to the left
    //"
    if (dsack_inserted)
        state->sacks_array.pop_front(); // delete DSACK entry

    // reset flags:
    state->snd_sack = false;
    state->snd_dsack = false;
    state->start_seqno = 0;
    state->end_seqno = 0;

    return *tcpHeader;
}

} // namespace tcp
} // namespace inet

