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

    // An ACK below SND.UNA (a late ACK) counts when its SACK blocks report new data,
    // so that the recovery uses that data, as Linux does on the old_ack path of
    // tcp_ack(); prrNewlyDelivered() gives the bytes that this segment SACKed first.
    if (seqLess(tcpHeader->getAckNo(), state->snd_una))
        return prrNewlyDelivered() > 0 && state->snd_una != state->snd_max;

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
    // overhead, or the second lost segment misses the recovery-entry burst.
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
        bool retransmission = seqLess(seqNum, state->snd_max);
        if (retransmission)
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
            if (sentBytes == 0) // no data left after the forward of snd_nxt
                break;

            // RFC 6937 accounting: sendSegment() is called here DIRECTLY (not via
            // sendData()/retransmitOneSegment()), so the dataSent()/segmentRetransmitted()
            // callbacks that feed prrOut never fire for these sends. Count them here, or
            // prrOut stays 0 and PRR's sndcnt = prrDelivered - prrOut over-sends.
            if (state->prrEnabled && state->lossRecovery)
                state->prrOut += sentBytes;

            // the loss undo counts every retransmission of the episode, also those
            // of step (C), which do not pass segmentRetransmitted() (Linux counts
            // each retransmitted skb in undo_retrans)
            if (retransmission && sentBytes > 0)
                countUndoRetransmission(seqNum, seqNum + sentBytes);

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
    // RFC 8985 section 7.1: "Reset TLP.is_retrans and TLP.end_seq when initiating a
    // connection, fast recovery, or RTO recovery."
    state->tlpHighSeq = 0;
    state->tlpRetrans = false;

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

    // Reduce cwnd/ssthresh per the connection's congestion-control flavour (Linux
    // icsk_ca_ops->ssthresh): the default is RFC 5681/6675's max(FlightSize/2, 2*SMSS)
    // (TcpAlgorithmBase::calculateSsthreshForFastRecovery), but CUBIC applies its own
    // beta (cwnd*0.7) -- hardcoding FlightSize/2 here gave CUBIC connections the wrong
    // post-recovery ssthresh (the fast_recovery/PRR scripts are all CUBIC). Capture the
    // pre-reduction cwnd first for PRR's priorCwnd (Linux tp->prior_cwnd = tp->snd_cwnd).
    // RFC 2883/3522: capture the undo context BEFORE the reduction below, so a
    // later D-SACK proving the retransmission spurious can restore cwnd/ssthresh.
    if (state->lossUndoEnabled)
        undoInit();
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
        // RFC 6675 section 5.1: after an RTO during a loss recovery, "a new recovery
        // phase (as described in Section 5) MUST NOT be initiated until HighACK is
        // greater than or equal to the new value of RecoveryPoint"
        if (isRecoveryBlockedAfterRto()) {
            EV_INFO << "No new loss recovery before the RecoveryPoint of the RTO (RFC 6675 section 5.1)\n";
            return;
        }

        //"
        // (1) If DupAcks >= DupThresh, go to step (4).
        //     Note: This check covers the case when a TCP receives SACK
        //     information for multiple segments smaller than SMSS, which can
        //     potentially prevent IsLost() (next step) from declaring a segment
        //     as lost.
        //"
        // RACK (RFC 8985, step 4 of the detection) replaces this count: it enters
        // the recovery itself when DupThresh segments are SACKed and no reordering
        // was seen, and not at all by DupThresh once reordering was seen (Linux
        // tcp_time_to_recover() skips the duplicate-ACK heuristic with RACK).
        // The RACK marks reach this function through IsLost() in step (2).
        if (state->lossDetectionMode != 1 && state->dupacks >= state->dupthresh)
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
                while (state->limited_transmit_enabled
                       && (int32_t)state->snd_cwnd - (int32_t)state->pipe >= (int32_t)state->snd_mss) {
                    // Limited Transmit (RFC 3042, RFC 6675 step 3.3) sends only previously
                    // unsent data, from HighData+1, in both loss detection modes. NextSeg() is
                    // not part of this step: its rule 3 returns an old unSACKed segment when
                    // no new data waits, which would retransmit the first hole before the
                    // recovery starts. Linux sends only new data before a recovery too.
                    uint32_t seqNum = state->snd_max;
                    if (seqLE(seqNum + state->snd_mss, state->snd_una + state->snd_wnd)) {
                        state->snd_nxt = seqNum;
                        uint32_t oldSndMax = state->snd_max;
                        uint32_t sentBytes = conn->sendSegment(state->snd_mss);
                        if (sentBytes == 0) // no data left after the forward of snd_nxt
                            break;
                        state->pipe += sentBytes;
                        state->limitedTransmitBytes += state->snd_max - oldSndMax;
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
                // than once. Record it for the loss undo (the RFC deliberately leaves
                // the action unspecified).
                noteDsack(tmp.getStart(), tmp.getEnd());
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
                    // RFC 2883: the segment identified by this block was received more
                    // than once. Record it for the loss undo (the RFC deliberately leaves
                    // the action unspecified).
                    noteDsack(tmp.getStart(), tmp.getEnd());
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
                uint32_t newlySackedLow = conn->getRexmitQueueForUpdate()->setSackedBit(tmp.getStart(), tmp.getEnd());
                // Reordering detection (Linux tcp_sacktag_one/tcp_check_sack_reordering):
                // a never-retransmitted range newly SACKed BELOW the FACK of an earlier
                // ACK proves the network delivered it out of order -- data above it
                // arrived first. The other blocks of this ACK do not count, because the
                // blocks come in the order of arrival, and RFC 8985 step 3 takes them in
                // ascending order. A re-reported or merely grown block returns
                // newlySackedLow at/above the prior FACK and is ignored, as are SACKs of
                // retransmissions.
                // F-RTO (SACK side): newly SACKed data that was never retransmitted
                // likewise proves the original flight arrived.
                if (state->frtoActive && newlySackedLow != 0)
                    state->frtoOrigAcked = true;
                if (state->adaptiveReorderingEnabled && newlySackedLow != 0
                        && seqLess(newlySackedLow, state->priorFack))
                    checkSackReordering(newlySackedLow);
            }
            else
                EV_DETAIL << "Received SACK below total cumulative ACK snd_una=" << state->snd_una << "\n";
        }
        // the loss marks that the count of the bytes in flight reads: the DupThresh
        // rule of RFC 6675, by segments. Loss marking is exclusive per mode (Linux
        // tcp_identify_packet_loss): under RACK only time-based marking below may set
        // lost -- the DupThresh region rule would pre-mark burst holes on the first
        // SACK, which both defeats the reordering-window timer (already-lost regions
        // are skipped as candidates) and over-counts small (sub-MSS) SACKed regions
        // against a segment threshold.
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
    // whose range was already discarded, or before anything is queued.
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
        // byte-range SACK of a bigger skb's tail never gets tagged and never
        // advances the kernel's RACK reference -- TLP fires there instead of a
        // RACK retransmit. A WHOLE small skb (e.g. a fully-SACKed 400B MSG_EOR
        // chunk) IS tagged and DOES advance RACK, so only the buffer-tail
        // fragment case is skipped.
        // A region that STARTS at a genuine transmission boundary is a whole
        // (small) segment, not a split-off fragment -- Linux tags it, so it
        // must advance the reference.
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
    // enters on the ACK itself.
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
        // range is sent once more.
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

void Rfc6675Recovery::onRexmitTimeout()
{
    // capture the undo context BEFORE the RTO's ssthresh/cwnd reduction
    bool frto = state->frtoEnabled && state->sack_enabled;
    if ((state->lossUndoEnabled || frto) && isNewLossEpisode())
        undoInit();

    // F-RTO (RFC 5682 section 3, SACK-enhanced): the timeout opens a detection
    // episode, but not during a fast recovery or during the recovery of an earlier
    // timeout (step 1: "If RecoveryPoint is larger than or equal to SND.UNA, do not
    // enter step 2"). A repeated timeout with no new ACK since the last one keeps
    // the episode open (step 2: "If the retransmission timeout expires again, go to
    // step 1").
    if (frto) {
        bool recoveryOpen = state->lossRecovery
                || (state->rtoRecoveryPoint != 0 && seqLess(state->snd_una, state->rtoRecoveryPoint));
        state->frtoActive = state->frtoActive || !recoveryOpen;
        if (!state->frtoActive)
            EV_DETAIL << "F-RTO: no detection, the timeout comes during a loss recovery\n";
        state->frtoOrigAcked = false;
    }

    // the recovery of this timeout ends when the ACK reaches the data sent before it
    state->rtoRecoveryPoint = state->snd_max != 0 ? state->snd_max : 1; // nonzero marker
}

void Rfc6675Recovery::processFrtoEpisode()
{
    // called at each ACK of new data; the first one after the timeout decides
    if (state->rtoRecoveryPoint != 0 && seqGE(state->snd_una, state->rtoRecoveryPoint))
        state->rtoRecoveryPoint = 0; // the recovery of the timeout is complete
    if (!state->frtoActive)
        return;
    state->frtoActive = false;
    if (state->frtoOrigAcked) {
        // RFC 5682 step 3.b, Linux FLAG_ORIG_SACK_ACKED: data that was sent only once
        // was (s)acked, so the original flight arrived and the timeout was spurious.
        // Restore the cwnd and ssthresh of before the timeout (Linux
        // tcp_try_undo_loss(frto_undo=true)) and forget the loss marks.
        EV_INFO << "F-RTO: spurious retransmission timeout detected, undoing the RTO response\n";
        undoCwndReduction();
        conn->getRexmitQueueForUpdate()->resetLostBit();
        state->afterRto = false;
        state->rexmit_count = 0; // Linux clears icsk_retransmits on the undo
        state->frtoOrigAcked = false;
    }
    else {
        // RFC 5682 step 2.b: the sender does not send new data here, so the episode
        // ends at the first ACK of new data, as RFC 5682 and Linux tcp_process_loss()
        // end it when no new data can go. The loss was real. The undo context stays
        // only for the D-SACK and Eifel undo.
        EV_DETAIL << "F-RTO: the first new ACK after the timeout acknowledges no data that was sent once, the loss is real\n";
        if (!state->lossUndoEnabled)
            state->undoMarker = 0;
    }
}

bool Rfc6675Recovery::isNewLossEpisode() const
{
    // Linux tcp_enter_loss(): a timeout starts a new undo context, except during a
    // fast recovery and at a repeated timeout with no new ACK since the last one
    // (TcpAlgorithmBase increments rexmit_count before this call, and an ACK of new
    // data resets it). These keep the context of the episode in progress. An older
    // context that no undo ended must not stay: its counts and its cwnd are stale.
    if (state->undoMarker == 0)
        return true;
    return !state->lossRecovery && state->rexmit_count == 1;
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

void Rfc6675Recovery::countUndoRetransmission(uint32_t fromSeq, uint32_t toSeq)
{
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

void Rfc6675Recovery::noteDsack(uint32_t fromSeq, uint32_t toSeq)
{
    state->dsackSeen = true;
    state->dsackBytes = toSeq - fromSeq;
    state->dsackEndSeq = toSeq;
    // Linux tcp_check_dsack()/tcp_sacktag_one(): each D-SACK of data above the undo
    // marker confirms that many retransmitted segments as spurious, on whatever ACK
    // it arrives -- usually a duplicate ACK, because the original arrived first.
    if (state->lossUndoEnabled && state->undoMarker != 0 && state->undoRetrans > 0
            && seqGreater(toSeq, state->undoMarker))
    {
        uint32_t segs = (state->dsackBytes + state->snd_mss - 1) / state->snd_mss;
        state->undoRetrans -= (int32_t)segs;
        if (state->undoRetrans < 0)
            state->undoRetrans = 0;
    }
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

void Rfc6675Recovery::reoTimeout()
{
    // RACK marked further bytes lost while no ACK was arriving. If we are not yet
    // recovering, this is the fast-retransmit trigger RACK exists to provide;
    // otherwise just push out whatever the scoreboard now says is missing.
    if (!state->lossRecovery) {
        if (!isRecoveryBlockedAfterRto()) // RFC 6675 section 5.1, as in receivedDuplicateAck()
            step4();
    }
    else
        stepC();
}

bool Rfc6675Recovery::isRecoveryBlockedAfterRto()
{
    if (state->recoveryAfterRtoBlocked && seqGE(state->snd_una, state->recoveryPoint))
        state->recoveryAfterRtoBlocked = false;
    return state->recoveryAfterRtoBlocked;
}

void Rfc6675Recovery::segmentsAcked(uint32_t fromSeq, uint32_t toSeq)
{
    // F-RTO (RFC 5682 sec 3.1 step 3.b, cumulative side): the scoreboard for
    // [fromSeq, toSeq) is still intact here. If any part of the newly
    // cumulatively-acked range was transmitted exactly once -- i.e. is NOT one of
    // the post-RTO retransmissions -- and was not SACKed before, then the original
    // flight (or part of it) reached the receiver after the RTO, so the timeout was
    // spurious. Data SACKed before the RTO is no new evidence (Linux
    // FLAG_ORIG_SACK_ACKED skips TCPCB_SACKED_ACKED segments).
    if (state->frtoActive && state->sack_enabled) {
        auto frq = conn->getRexmitQueue();
        if (frq != nullptr && frq->getQueueLength() > 0) {
            for (uint32_t seq = std::max(fromSeq, frq->getBufferStartSeq());
                 seqLess(seq, std::min(toSeq, frq->getBufferEndSeq())); )
            {
                const auto& region = frq->getRegion(seq);
                if (region.transmitCount <= 1 && !region.everSacked) {
                    state->frtoOrigAcked = true;
                    break;
                }
                seq = region.endSeqNum;
            }
        }
    }
    processFrtoEpisode();

    // Adaptive reordering: if this cumulatively-acked segment was never retransmitted
    // yet sits below data that an earlier ACK SACKed, it was merely reordered (not lost)
    // -- grow the learned reordering degree so it stops causing spurious fast
    // retransmits. The SACK blocks of this ACK do not count: the receiver can SACK
    // later data and acknowledge this segment in one ACK (RFC 8985 step 3 takes the
    // newly acknowledged segments in ascending order, Linux compares with prior_fack).
    if (state->adaptiveReorderingEnabled && state->sack_enabled && seqLess(fromSeq, state->priorFack)) {
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

    countUndoRetransmission(fromSeq, toSeq);
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
    uint32_t length = 0;

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
    // scoreboard actually covers, or the walk runs off its end and aborts
    // (seen on a TFO fallback that closes with data still in flight).
    //
    // One iterator walks the scoreboard: a search from the front for each run of
    // octets, in checkSackBlock() and in isLost() of RACK mode, made the walk
    // quadratic in the number of regions. Each step still covers the rest of one
    // region, so the sums and their order stay the same.
    auto rexmitQueue = conn->getRexmitQueue();
    uint32_t scanEnd = rexmitQueue->getBufferEndSeq();
    if (seqLess(state->snd_max, scanEnd))
        scanEnd = state->snd_max;
    ASSERT(seqLE(rexmitQueue->getBufferStartSeq(), state->snd_una) || !seqLess(state->snd_una, scanEnd));
    auto region = rexmitQueue->rexmitQueue.begin();
    for (uint32_t s1 = state->snd_una; seqLess(s1, scanEnd); s1 += length) {
        while (region != rexmitQueue->rexmitQueue.end() && seqLE(region->endSeqNum, s1))
            region++;
        ASSERT(region != rexmitQueue->rexmitQueue.end() && seqLE(region->beginSeqNum, s1));
        length = region->endSeqNum - s1;
        bool sacked = region->sacked;

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
            // in RACK mode, isLost() is the lost flag of the region of S1
            bool lost = state->lossDetectionMode == 1 ? region->lost : isLost(s1);
            if (lost == false)
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
    // retransmit died).
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

} // namespace tcp
} // namespace inet

