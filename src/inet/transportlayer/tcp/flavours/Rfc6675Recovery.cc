//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

// 5. Algorithm Details
//    Upon the receipt of any ACK containing SACK information, the
//    scoreboard MUST be updated via the Update () routine.
// implemented in TcpConnection::processSACKOption()
//
//    If the incoming ACK is a cumulative acknowledgment, the TCP MUST
//    reset DupAcks to zero.
// implemented in TcpConnection::processSegment1stThru8th() and TcpConnection::processAckInEstabEtc()

void Rfc6675Recovery::stepA()
{
    // (A) An incoming cumulative ACK for a sequence number greater than
    //     RecoveryPoint signals the end of loss recovery, and the loss
    //     recovery phase MUST be terminated.  Any information contained in
    //     the scoreboard for sequence numbers greater than the new value of
    //     HighACK SHOULD NOT be cleared when leaving the loss recovery
    //     phase.
    if (seqGreater(state->snd_una, state->recoveryPoint)) {
        state->lossRecovery = false;
        conn->getRexmitQueueForUpdate()->discardUpTo(state->snd_una);
    }
}

void Rfc6675Recovery::stepB()
{
    // (B) Upon receipt of an ACK that does not cover RecoveryPoint, the
    //     following actions MUST be taken:
    if (seqLE(state->snd_una, state->recoveryPoint)) {
        // (B.1) Use Update () to record the new SACK information conveyed
        //       by the incoming ACK.
        // implemented by TcpConnection::processSACKOption()

        // (B.2) Use SetPipe () to re-calculate the number of octets still
        //       in the network.
        conn->setPipe();
    }
}

void Rfc6675Recovery::stepC()
{
    // (C) If cwnd - pipe >= 1 SMSS, the sender SHOULD transmit one or more
    //     segments as follows:
    while ((int32_t)state->snd_cwnd - (int32_t)state->pipe >= (int32_t)state->snd_mss) {
        // (C.1) The scoreboard MUST be queried via NextSeg () for the
        //       sequence number range of the next segment to transmit (if
        //       any), and the given segment sent.  If NextSeg () returns
        //       failure (no data to send), return without sending anything
        //       (i.e., terminate steps C.1 -- C.5).
        uint32_t seqNum;
        if (!conn->nextSeg(seqNum))
            break;

        // (C.2) If any of the data octets sent in (C.1) are below HighData,
        //       HighRxt MUST be set to the highest sequence number of the
        //       retransmitted segment unless NextSeg () rule (4) was
        //       invoked for this retransmission.
        if (seqLess(seqNum, state->snd_max))
            state->highRxt = seqNum + state->snd_mss;

        // (C.3) If any of the data octets sent in (C.1) are above HighData,
        //       HighData must be updated to reflect the transmission of
        //       previously unsent data.
        if (seqGreater(seqNum, state->snd_max))
            state->snd_max = seqNum + state->snd_mss;

        if (seqLE(seqNum + state->snd_mss, state->snd_una + state->snd_wnd)) {
            state->snd_nxt = seqNum;
            uint32_t sentBytes = conn->sendSegment(state->snd_mss);

            // (C.4) The estimate of the amount of data outstanding in the
            //       network must be updated by incrementing pipe by the number
            //       of octets transmitted in (C.1).
            state->pipe += sentBytes;
        }
        else
            break;

        // (C.5) If cwnd - pipe >= 1 SMSS, return to (C.1)
    }
}

void Rfc6675Recovery::receivedDataAck(uint32_t firstSeqAcked)
{
    // Once a TCP is in the loss recovery phase, the following procedure
    // MUST be used for each arriving ACK:
    if (state->lossRecovery) {
        stepA();
        stepB();
        stepC();
        // Note that steps (A) and (C) can potentially send a burst of
        // back-to-back segments into the network if the incoming cumulative
        // acknowledgment is for more than SMSS octets of data, or if incoming
        // SACK blocks indicate that more than SMSS octets of data have been
        // lost in the second half of the window.
    }
}

void Rfc6675Recovery::step4()
{
    // (4) Invoke fast retransmit and enter loss recovery as follows:
    state->lossRecovery = true;

    // (4.1) RecoveryPoint = HighData
    //       When the TCP sender receives a cumulative ACK for this data
    //       octet, the loss recovery phase is terminated.
    state->recoveryPoint = state->snd_max;

    // (4.2) ssthresh = cwnd = (FlightSize / 2)
    //       The congestion window (cwnd) and slow start threshold
    //       (ssthresh) are reduced to half of FlightSize per [RFC5681].
    //       Additionally, note that [RFC5681] requires that any
    //       segments sent as part of the Limited Transmit mechanism not
    //       be counted in FlightSize for the purpose of the above
    //       equation.
    state->ssthresh = state->snd_cwnd = conn->getBytesInFlight() / 2;

    // (4.3) Retransmit the first data segment presumed dropped -- the
    //       segment starting with sequence number HighACK + 1.  To
    //       prevent repeated retransmission of the same data or a
    //       premature rescue retransmission, set both HighRxt and
    //       RescueRxt to the highest sequence number in the
    //       retransmitted segment.
    // TODO which one and why?
    conn->retransmitOneSegment(false);
    // conn->sendDataDuringLossRecoveryPhase(status->snd_cwnd);

    // (4.4) Run SetPipe ()
    //       Set a "pipe" variable to the number of outstanding octets
    //       currently "in the pipe"; this is the data which has been
    //       sent by the TCP sender but for which no cumulative or
    //       selective acknowledgment has been received and the data has
    //       not been determined to have been dropped in the network.
    //       It is assumed that the data is still traversing the network
    //       path.
    conn->setPipe();

    // (4.5) In order to take advantage of potential additional
    //       available cwnd, proceed to step (C) below.
    stepC();
}

void Rfc6675Recovery::receivedDuplicateAck()
{
    // If the incoming ACK is a duplicate acknowledgment per the definition
    // in Section 2 (regardless of its status as a cumulative
    // acknowledgment), and the TCP is not currently in loss recovery, the
    // TCP MUST increase DupAcks by one and take the following steps:
    if (!state->lossRecovery) {
        // (1) If DupAcks >= DupThresh, go to step (4).
        //     Note: This check covers the case when a TCP receives SACK
        //     information for multiple segments smaller than SMSS, which can
        //     potentially prevent IsLost() (next step) from declaring a segment
        //     as lost.
        if (state->dupacks >= state->dupthresh)
            step4();
        else {
            // (2) If DupAcks < DupThresh but IsLost (HighACK + 1) returns true --
            //     indicating at least three segments have arrived above the current
            //     cumulative acknowledgment point, which is taken to indicate loss
            //     -- go to step (4).
            if (conn->isLost(state->snd_una + 1))
                step4();
            else {
                // (3) The TCP MAY transmit previously unsent data segments as per
                //     Limited Transmit [RFC5681], except that the number of octets
                //     which may be sent is governed by pipe and cwnd as follows:

                // (3.1) Set HighRxt to HighACK.
                state->highRxt = state->snd_una;

                // (3.2) Run SetPipe ().
                conn->setPipe();

                // (3.3) If (cwnd - pipe) >= 1 SMSS, there exists previously unsent
                //       data, and the receiver's advertised window allows, transmit
                //       up to 1 SMSS of data starting with the octet HighData+1 and
                //       update HighData to reflect this transmission, then return
                //       to (3.2).
                while ((int32_t)state->snd_cwnd - (int32_t)state->pipe >= (int32_t)state->snd_mss) {
                    uint32_t seqNum;
                    if (!conn->nextSeg(seqNum))
                        break;
                    if (seqLE(seqNum + state->snd_mss, state->snd_una + state->snd_wnd)) {
                        state->snd_nxt = seqNum;
                        uint32_t sentBytes = conn->sendSegment(state->snd_mss);
                        state->pipe += sentBytes;
                    }
                    else
                        break;
                }

                // (3.4) Terminate processing of this ACK.
            }
        }
    }
    else {
        stepA();
        stepB();
        stepC();
    }
}

} // namespace tcp
} // namespace inet

