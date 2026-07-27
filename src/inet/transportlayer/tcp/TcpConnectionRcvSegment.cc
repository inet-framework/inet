//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2011 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <string.h>

#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

bool TcpConnection::tryFastRoute(const Ptr<const TcpHeader>& tcpHeader)
{
    // fast route processing not yet implemented
    return false;
}

void TcpConnection::segmentArrivalWhileClosed(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    EV_INFO << "Seg arrived: ";
    printSegmentBrief(tcpSegment, tcpHeader);

    // This segment doesn't belong to any connection, so this object
    // must be a temp object created solely for the purpose of calling us

    ASSERT(state == nullptr);

    EV_INFO << "Segment doesn't belong to any existing connection\n";

    // RFC 793:
    //"
    // all data in the incoming segment is discarded.  An incoming
    // segment containing a RST is discarded.  An incoming segment not
    // containing a RST causes a RST to be sent in response.  The
    // acknowledgment and sequence field values are selected to make the
    // reset sequence acceptable to the TCP that sent the offending
    // segment.
    //
    // If the ACK bit is off, sequence number zero is used,
    //
    //    <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK>
    //
    // If the ACK bit is on,
    //
    //    <SEQ=SEG.ACK><CTL=RST>
    //
    // ...
    //
    //    SEG.LEN = the number of octets occupied by the data in the segment
    //              (counting SYN and FIN)
    //"
    if (tcpHeader->getRstBit()) {
        EV_DETAIL << "RST bit set: dropping segment\n";
        return;
    }

    if (!tcpHeader->getAckBit()) {
        EV_DETAIL << "ACK bit not set: sending RST+ACK\n";
        uint32_t ackNo = tcpHeader->getSequenceNo() + tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>() + tcpHeader->getSynFinLen();
        sendRstAck(0, ackNo, destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
    }
    else {
        EV_DETAIL << "ACK bit set: sending RST\n";
        sendRst(tcpHeader->getAckNo(), destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
    }
}

TcpEventCode TcpConnection::process_RCV_SEGMENT(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address src, L3Address dest)
{
    EV_INFO << "Seg arrived: ";
    printSegmentBrief(tcpSegment, tcpHeader);
    EV_DETAIL << "TCB: " << state->str() << "\n";

    state->time_last_segment_received = simTime(); // idle base for keepalive

    // snapshot delivered-bytes so consumers can read this segment's newly
    // acked+sacked bytes as deliveredBytes - prrDeliveredMark (RFC 6937 PRR input,
    // also used by AccECN to approximate this ACK's delivered packet count)
    state->prrDeliveredMark = state->deliveredBytes;

    // reset the per-segment D-SACK detection (RFC 2883 loss undo)
    state->dsackSeen = false;
    state->dsackBytes = 0;

    emit(rcvSeqSignal, tcpHeader->getSequenceNo());
    emit(rcvAckSignal, tcpHeader->getAckNo());

    emit(tcpRcvPayloadBytesSignal, int(tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>()));
    //
    // Note: this code is organized exactly as
    // RFC 9293, section "3.10 Event Processing", subsection "3.10.7. SEGMENT ARRIVES".
    //
    TcpEventCode event;

    if (fsm.getState() == TCP_S_LISTEN) {
        event = processSegmentInListen(tcpSegment, tcpHeader, src, dest);
    }
    else if (fsm.getState() == TCP_S_SYN_SENT) {
        event = processSegmentInSynSent(tcpSegment, tcpHeader, src, dest);
    }
    else {
        // RFC 793 steps "first check sequence number", "second check the RST bit", etc
        event = processSegment1stThru8th(tcpSegment, tcpHeader);
    }

    delete tcpSegment;
    return event;
}

bool TcpConnection::hasEnoughSpaceForSegmentInReceiveQueue(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    // TODO must rewrite it
//    return (state->freeRcvBuffer >= tcpHeader->getPayloadLength()); // enough freeRcvBuffer in rcvQueue for new segment?

    long int payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();
    uint32_t payloadSeq = tcpHeader->getSequenceNo();
    uint32_t firstSeq = receiveQueue->getFirstSeqNo();
    if (seqLess(payloadSeq, firstSeq)) {
        long delta = firstSeq - payloadSeq;
        payloadSeq += delta;
        payloadLength -= delta;
    }
    return seqLE(firstSeq, payloadSeq) && seqLE(payloadSeq + payloadLength, firstSeq + state->maxRcvBuffer);
}

TcpEventCode TcpConnection::processSegment1stThru8th(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{

    // Delegates additional processing of ECN to the algorithm
    tcpAlgorithm->processEcnInEstablished();

    //
    // RFC 793: first check sequence number
    //

    bool acceptable = true;

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) { // Header options present? TCP_HEADER_OCTETS = 20
        // PAWS
        if (state->ts_enabled) {
            uint32_t tsval = getTSval(tcpHeader);
            if (tsval != 0 && seqLess(tsval, state->ts_recent) &&
                (simTime() - state->time_last_data_sent) > PAWS_IDLE_TIME_THRESH) // PAWS_IDLE_TIME_THRESH = 24 days
            {
                EV_DETAIL << "PAWS: Segment is not acceptable, TSval=" << tsval << " in "
                          << stateName(fsm.getState()) << " state received: dropping segment\n";
                acceptable = false;
            }
        }

        readHeaderOptions(tcpHeader);
    }

    if (acceptable)
        acceptable = isSegmentAcceptable(tcpSegment, tcpHeader);

    int payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();

    if (!acceptable) {
        //"
        // If an incoming segment is not acceptable, an acknowledgment
        // should be sent in reply (unless the RST bit is set, if so drop
        // the segment and return):
        //
        //  <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
        //"
        if (tcpHeader->getRstBit()) {
            EV_DETAIL << "RST with unacceptable seqNum: dropping\n";
        }
        else {
            if (tcpHeader->getSynBit()) {
                EV_DETAIL << "SYN with unacceptable seqNum in " << stateName(fsm.getState()) << " state received (SYN duplicat?)\n";
                // Only a PURE SYN retransmit earns the SYN-ACK resend (Linux
                // tcp_check_req's request-socket path). A SYN+ACK here is the
                // peer completing a SIMULTANEOUS open -- it takes the normal
                // dup-segment route below: a D-SACK-bearing plain ACK, never a
                // SYN-ACK retransmit (simultaneous-fast-open pins
                // ". 1001:1001(0) ack 1 <sack 0:1>").
                if (fsm.getState() == TCP_S_SYN_RCVD && !tcpHeader->getAckBit()) {
                    // A retransmitted SYN while we sit in SYN_RCVD means our
                    // SYN-ACK was lost: Linux re-sends the SYN-ACK (from the
                    // ORIGINAL negotiation -- tcp_check_req/TCP_SYN_RECV
                    // resend path), not a plain ACK. The AccECN
                    // accecn_then_notecn_syn / notecn_then_accecn_syn pair
                    // pins both directions: renegotiating from the new SYN's
                    // (possibly different) ECN codepoint would be wrong, so
                    // the stored negotiation state is reused as-is.
                    EV_DETAIL << "Re-sending SYN-ACK for the retransmitted SYN\n";
                    // count it as a SYN-ACK retransmission: the rexmit-gated
                    // option rules apply (e.g. Linux omits the AccECN option
                    // on SYN-ACK retransmits -- the *_drop/_rxmt scripts and
                    // accecn_then_notecn_syn pin this)
                    state->syn_rexmit_count++;
                    sendSynAck();
                    // AccECN downgrade (accecn_then_notecn_syn): a peer whose
                    // RETRANSMITTED SYN carries no ACE bits abandoned its ECN
                    // request (likely blackholed) -- stop setting ECT on our
                    // packets from here on (Linux ACE_FAIL handling), while
                    // the ACE-field/option feedback machinery keeps running.
                    // AFTER sendSynAck(): it re-derives ect from the stored
                    // negotiation and would undo the downgrade.
                    if (state->accEcnNegotiated && !tcpHeader->getAeBit()
                        && !tcpHeader->getEceBit() && !tcpHeader->getCwrBit() && state->ect)
                    {
                        EV_DETAIL << "Retransmitted SYN lost its ACE bits: disabling ECT marking\n";
                        state->ect = false;
                    }
                    state->rcv_naseg++;
                    emit(rcvNASegSignal, state->rcv_naseg);
                    return TCP_E_IGNORE;
                }
            }
            else if (payloadLength + tcpHeader->getSynFinLen() > 0 && state->sack_enabled
                     && seqLess(tcpHeader->getSequenceNo(), state->rcv_nxt)) {
                // Linux tcp_send_dupack: ANY old data (seq before rcv_nxt) earns a
                // D-SACK, including a duplicate ending exactly at rcv_nxt -- the
                // range's right edge is capped at rcv_nxt by addSacks. SEG.LEN
                // counts SYN and FIN (Linux end_seq): a duplicate SYN-ACK's
                // one-sequence-number SYN gets D-SACKed as [irs, irs+1)
                // (simultaneous-fast-open's "sack 0:1").
                //
                // Linux tcp_rcv_spurious_retrans, AccECN arm: our previous ACK for
                // this very duplicate carried both the AccECN option and its D-SACK,
                // yet the same segment arrives yet again -- a middlebox is evidently
                // dropping our option-bearing ACKs, so stop sending the option for
                // the rest of the connection (kernel:tcp_accecn_client_accecn_options_drop).
                if (state->accEcnNegotiated && state->accEcnOptSentWithDsack
                        && tcpHeader->getSequenceNo() == state->accEcnSentDsackStart
                        && !state->accEcnOptFailSend)
                {
                    EV_DETAIL << "AccECN option + D-SACK ACK was evidently lost twice: disabling AccECN option emission\n";
                    state->accEcnOptFailSend = true;
                }
                state->start_seqno = tcpHeader->getSequenceNo();
                state->end_seqno = tcpHeader->getSequenceNo() + payloadLength + tcpHeader->getSynFinLen();
                state->snd_dsack = true;
                EV_DETAIL << "SND_D-SACK SET (dupseg rcvd)\n";
            }

            EV_DETAIL << "Segment seqNum not acceptable, sending ACK with current receive seq\n";
            // RFC 2018, page 4:
            // "The receiver SHOULD send an ACK for every valid segment that arrives
            // containing new data, and each of these "duplicate" ACKs SHOULD bear a
            // SACK option."
            //
            // The received segment is not "valid" therefore the ACK will not bear a SACK option, if snd_dsack (D-SACK) is not set.
            sendAck();
        }

        state->rcv_naseg++;

        emit(rcvNASegSignal, state->rcv_naseg);

        return TCP_E_IGNORE;
    }

    // ECN
    if (tcpHeader->getCwrBit() == true) {
        EV_INFO << "Received CWR... Leaving ecnEcho State\n";
        state->ecnEchoState = false;
    }

    //
    // RFC 793: second check the RST bit,
    //
    if (tcpHeader->getRstBit()) {
        // Note: if we come from LISTEN, processSegmentInListen() has already handled RST.
        switch (fsm.getState()) {
            case TCP_S_SYN_RCVD:
                //"
                // If this connection was initiated with a passive OPEN (i.e.,
                // came from the LISTEN state), then return this connection to
                // LISTEN state and return.  The user need not be informed.  If
                // this connection was initiated with an active OPEN (i.e., came
                // from SYN-SENT state) then the connection was refused, signal
                // the user "connection refused".  In either case, all segments
                // on the retransmission queue should be removed.  And in the
                // active OPEN case, enter the CLOSED state and delete the TCB,
                // and return.
                //"
                return processRstInSynReceived(tcpHeader);

            case TCP_S_ESTABLISHED:
            case TCP_S_FIN_WAIT_1:
            case TCP_S_FIN_WAIT_2:
            case TCP_S_CLOSE_WAIT:
                //"
                // If the RST bit is set then, any outstanding RECEIVEs and SEND
                // should receive "reset" responses.  All segment queues should be
                // flushed.  Users should also receive an unsolicited general
                // "connection reset" signal.
                //
                // Enter the CLOSED state, delete the TCB, and return.
                //"
                EV_DETAIL << "RST: performing connection reset, closing connection\n";
                sendIndicationToApp(TCP_I_CONNECTION_RESET);
                return TCP_E_RCV_RST; // this will trigger state transition

            case TCP_S_CLOSING:
            case TCP_S_LAST_ACK:
            case TCP_S_TIME_WAIT:
                //"
                // enter the CLOSED state, delete the TCB, and return.
                //"
                EV_DETAIL << "RST: closing connection\n";
                return TCP_E_RCV_RST; // this will trigger state transition

            default:
                ASSERT(0);
                break;
        }
    }

    // RFC 793: third check security and precedence
    // This step is ignored.

    //
    // RFC 793: fourth, check the SYN bit,
    //
    if (tcpHeader->getSynBit()
            && !(fsm.getState() == TCP_S_SYN_RCVD && tcpHeader->getAckBit())) {
        //"
        // If the SYN is in the window it is an error, send a reset, any
        // outstanding RECEIVEs and SEND should receive "reset" responses,
        // all segment queues should be flushed, the user should also
        // receive an unsolicited general "connection reset" signal, enter
        // the CLOSED state, delete the TCB, and return.
        //
        // If the SYN is not in the window this step would not be reached
        // and an ack would have been sent in the first step (sequence
        // number check).
        //"
        // Zoltan Bojthe: but accept SYN+ACK in SYN_RCVD state for simultaneous open

        ASSERT(isSegmentAcceptable(tcpSegment, tcpHeader)); // assert SYN is in the window
        EV_DETAIL << "SYN is in the window: performing connection reset, closing connection\n";
        sendIndicationToApp(TCP_I_CONNECTION_RESET);
        return TCP_E_RCV_UNEXP_SYN;
    }

    //
    // RFC 793: fifth check the ACK field,
    //
    if (!tcpHeader->getAckBit()) {
        // if the ACK bit is off drop the segment and return
        EV_INFO << "ACK not set, dropping segment\n";
        return TCP_E_IGNORE;
    }

    uint32_t old_snd_una = state->snd_una;

    TcpEventCode event = TCP_E_IGNORE;

    if (fsm.getState() == TCP_S_SYN_RCVD) {
        //"
        // If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state
        // and continue processing.
        //
        // If the segment acknowledgment is not acceptable, form a
        // reset segment,
        //
        //  <SEQ=SEG.ACK><CTL=RST>
        //
        // and send it.
        //"
        if (!seqLE(state->snd_una, tcpHeader->getAckNo()) || !seqLE(tcpHeader->getAckNo(), state->snd_nxt)) {
            sendRst(tcpHeader->getAckNo());
            return TCP_E_IGNORE;
        }

        state->snd_effmss = calculateEffectiveMss();

        // Seed the RTT estimator from the handshake RTT (Linux measures the
        // SYN<->SYN-ACK exchange via tcp_ack_update_rtt/tcp_synack_rtt_meas and
        // enters ESTABLISHED with srtt/rttvar -- and hence the first RTO -- already
        // RTT-scaled instead of the initial default). Karn: skipped if our handshake
        // segment was retransmitted.
        if (state->seedRttFromHandshake && state->syn_rexmit_count == 0 && state->handshakeSentTime >= SIMTIME_ZERO)
            tcpAlgorithm->rttMeasurementComplete(state->handshakeSentTime, simTime());

        // notify tcpAlgorithm and app layer
        tcpAlgorithm->established(false);

        if (isToBeAccepted())
            sendAvailableIndicationToApp();
        else
            sendEstabIndicationToApp();

        // Simultaneous open completed by a duplicate SYN-ACK: its SYN occupies
        // an already-received sequence number, and Linux's tcp_data_queue
        // D-SACKs that one-sequence-number range with an immediate ACK right
        // after establishing (simultaneous-fast-open pins
        // ". 1001:1001(0) ack 1 <sack 0:1>").
        if (tcpHeader->getSynBit() && state->sack_enabled && state->dsack_enabled
                && seqLess(tcpHeader->getSequenceNo(), state->rcv_nxt)) {
            state->start_seqno = tcpHeader->getSequenceNo();
            state->end_seqno = tcpHeader->getSequenceNo() + 1;
            state->snd_dsack = true;
            state->ack_now = true;
            sendAck();
        }

        // This will trigger transition to ESTABLISHED. Timers and notifying
        // app will be taken care of in stateEntered().
        event = TCP_E_RCV_ACK;
    }

    uint32_t old_snd_nxt = state->snd_nxt; // later we'll need to see if snd_nxt changed
    // Note: If one of the last data segments is lost while already in LAST-ACK state (e.g. if using TCPEchoApps)
    // TCP must be able to process acceptable acknowledgments, however please note RFC 793, page 73:
    // "LAST-ACK STATE
    //    The only thing that can arrive in this state is an
    //    acknowledgment of our FIN.  If our FIN is now acknowledged,
    //    delete the TCB, enter the CLOSED state, and return."
    if (fsm.getState() == TCP_S_SYN_RCVD || fsm.getState() == TCP_S_ESTABLISHED ||
        fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2 ||
        fsm.getState() == TCP_S_CLOSE_WAIT || fsm.getState() == TCP_S_CLOSING ||
        fsm.getState() == TCP_S_LAST_ACK)
    {
        //
        // ESTABLISHED processing:
        //"
        //  If SND.UNA < SEG.ACK =< SND.NXT then, set SND.UNA <- SEG.ACK.
        //  Any segments on the retransmission queue which are thereby
        //  entirely acknowledged are removed.  Users should receive
        //  positive acknowledgments for buffers which have been SENT and
        //  fully acknowledged (i.e., SEND buffer should be returned with
        //  "ok" response).  If the ACK is a duplicate
        //  (SEG.ACK < SND.UNA), it can be ignored.  If the ACK acks
        //  something not yet sent (SEG.ACK > SND.NXT) then send an ACK,
        //  drop the segment, and return.
        //
        //  If SND.UNA < SEG.ACK =< SND.NXT, the send window should be
        //  updated.  If (SND.WL1 < SEG.SEQ or (SND.WL1 = SEG.SEQ and
        //  SND.WL2 =< SEG.ACK)), set SND.WND <- SEG.WND, set
        //  SND.WL1 <- SEG.SEQ, and set SND.WL2 <- SEG.ACK.
        //
        //  Note that SND.WND is an offset from SND.UNA, that SND.WL1
        //  records the sequence number of the last segment used to update
        //  SND.WND, and that SND.WL2 records the acknowledgment number of
        //  the last segment used to update SND.WND.  The check here
        //  prevents using old segments to update the window.
        //"
        bool ok = processAckInEstabEtc(tcpSegment, tcpHeader);

        if (!ok)
            return TCP_E_IGNORE; // if acks something not yet sent, drop it
    }

    if ((fsm.getState() == TCP_S_FIN_WAIT_1 && state->fin_ack_rcvd)) {
        //"
        // FIN-WAIT-1 STATE
        //   In addition to the processing for the ESTABLISHED state, if
        //   our FIN is now acknowledged then enter FIN-WAIT-2 and continue
        //   processing in that state.
        //"
        event = TCP_E_RCV_ACK; // will trigger transition to FIN-WAIT-2
    }

    if (fsm.getState() == TCP_S_FIN_WAIT_2) {
        //"
        // FIN-WAIT-2 STATE
        //  In addition to the processing for the ESTABLISHED state, if
        //  the retransmission queue is empty, the user's CLOSE can be
        //  acknowledged ("ok") but do not delete the TCB.
        //"
        // nothing to do here (in our model, used commands don't need to be
        // acknowledged)
    }

    if (fsm.getState() == TCP_S_CLOSING) {
        //"
        // In addition to the processing for the ESTABLISHED state, if
        // the ACK acknowledges our FIN then enter the TIME-WAIT state,
        // otherwise ignore the segment.
        //"
        if (state->fin_ack_rcvd) {
            EV_INFO << "Our FIN acked -- can go to TIME_WAIT now\n";
            event = TCP_E_RCV_ACK; // will trigger transition to TIME-WAIT
            scheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer); // start timer

            // we're entering TIME_WAIT, so we can signal CLOSED the user
            // (the only thing left to do is wait until the 2MSL timer expires)
        }
    }

    if (fsm.getState() == TCP_S_LAST_ACK) {
        //"
        // The only thing that can arrive in this state is an
        // acknowledgment of our FIN.  If our FIN is now acknowledged,
        // delete the TCB, enter the CLOSED state, and return.
        //"
        if (state->send_fin && tcpHeader->getAckNo() == state->snd_fin_seq + 1) {
            EV_INFO << "Last ACK arrived\n";
            return TCP_E_RCV_ACK; // will trigger transition to CLOSED
        }
    }

    if (fsm.getState() == TCP_S_TIME_WAIT) {
        //"
        // The only thing that can arrive in this state is a
        // retransmission of the remote FIN.  Acknowledge it, and restart
        // the 2 MSL timeout.
        //"
        // And we are staying in the TIME_WAIT state.
        //
        sendAck();
        rescheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);
    }

    //
    // RFC 793: sixth, check the URG bit,
    //
    if (tcpHeader->getUrgBit() && (fsm.getState() == TCP_S_ESTABLISHED ||
                                   fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2))
    {
        //"
        // If the URG bit is set, RCV.UP <- max(RCV.UP,SEG.UP), and signal
        // the user that the remote side has urgent data if the urgent
        // pointer (RCV.UP) is in advance of the data consumed.  If the
        // user has already been signaled (or is still in the "urgent
        // mode") for this continuous sequence of urgent data, do not
        // signal the user again.
        //"

        // TODO URG currently not supported
    }

    //
    // RFC 9293: Seventh, process the segment text
    //
    uint32_t old_rcv_nxt = state->rcv_nxt; // if rcv_nxt changes, we need to send/schedule an ACK
    // D-SACK bookkeeping (RFC 2883): first already-buffered range duplicated by
    // this segment, captured just before the insert merges the regions.
    bool dupRangeFound = false;
    uint32_t dupStart = 0, dupEnd = 0;

    // RFC 1122 4.2.2.13 (Linux TCPABORTONDATA, tcp_rcv_state_process): the
    // application has fully CLOSEd -- it will never read again -- so NEW data
    // arriving in FIN_WAIT_1/2 would be silently discarded while the peer
    // believes it was delivered. RFC 793 said queue it; RFC 1122 (and BSD,
    // and Linux) say reset the connection instead. Only after a FULL close
    // (rcvShutdown): a shutdown(SHUT_WR) half close keeps receiving legal.
    // The FIN bit itself is not data for this test, and the RST is the
    // reset-REPLY form (Linux returns 1 from tcp_rcv_state_process and
    // tcp_v4_send_reset stamps the RST from the offending segment's ack
    // field, not from snd_nxt -- user_timeout pins 'R <ackNo>').
    if ((fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2)
        && state->rcvShutdown && payloadLength > 0
        && seqGreater(tcpHeader->getSequenceNo() + payloadLength, state->rcv_nxt))
    {
        EV_INFO << "New data after full CLOSE (application gone) -- resetting the connection (RFC 1122 4.2.2.13)\n";
        sendRst(tcpHeader->getAckNo());
        return TCP_E_ABORT;
    }

    if (fsm.getState() == TCP_S_SYN_RCVD || fsm.getState() == TCP_S_ESTABLISHED ||
        fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2)
    {
        //"
        // Once in the ESTABLISHED state, it is possible to deliver segment
        // data to user RECEIVE buffers.  Data from segments can be moved
        // into buffers until either the buffer is full or the segment is
        // empty.  If the segment empties and carries a PUSH flag, then
        // the user is informed, when the buffer is returned, that a PUSH
        // has been received.
        //
        // When the TCP takes responsibility for delivering the data to the
        // user, it must also acknowledge the receipt of the data.
        //
        // Once the TCP takes responsibility for the data, it advances
        // RCV.NXT over the data accepted, and adjusts RCV.WND as
        // appropriate to the current buffer availability.  The total of
        // RCV.NXT and RCV.WND should not be reduced.
        //
        // A TCP implementation MAY send an ACK segment acknowledging
        // RCV.NXT when a valid segment arrives that is in the window but
        // not at the left window edge (MAY-13).

        // Please note the window management suggestions in section 3.8.
        //
        // Send an acknowledgment of the form:
        //
        //   <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
        //
        // This acknowledgment should be piggybacked on a segment being
        // transmitted if possible without incurring undue delay.
        //"

        if (payloadLength > 0) {
            // check for full sized segment
            if ((uint32_t)payloadLength == state->snd_mss || (uint32_t)payloadLength + (tcpHeader->getHeaderLength() - TCP_MIN_HEADER_LENGTH).get<B>() == state->snd_mss)
                state->full_sized_segment_counter++;

            // check for persist probe
            if (payloadLength == 1)
                state->ack_now = true; // TODO how to check if it is really a persist probe?

            updateRcvQueueVars();

            if (hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) { // enough freeRcvBuffer in rcvQueue for new segment?
                EV_DETAIL << "Processing segment text in a data transfer state\n";

                // insert into receive buffers. If this segment is contiguous with
                // previously received ones (seqNo == rcv_nxt), rcv_nxt can be increased;
                // otherwise it stays the same but the data must be cached nevertheless
                // (to avoid "Failure to retain above-sequence data" problem, RFC 2525
                // section 2.5).

                uint32_t old_usedRcvBuffer = state->usedRcvBuffer;
                // D-SACK (RFC 2883): find the first already-buffered range this
                // segment duplicates BEFORE the insert merges the regions (Linux
                // reports it via tcp_dsack_set/tcp_dsack_extend as tcp_ofo_queue
                // drains the out-of-order queue over a gap-filling segment).
                if (state->sack_enabled && state->dsack_enabled && payloadLength > 0)
                    dupRangeFound = receiveQueue->findFirstDuplicateRange(tcpHeader->getSequenceNo(),
                            tcpHeader->getSequenceNo() + payloadLength, dupStart, dupEnd);
                state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, tcpHeader);

                // Receive-buffer occupancy at Linux's skb-truesize granularity,
                // plus the measured payload/truesize scaling ratio
                // (tcp_measure_rcv_mss: updated when a segment at least as
                // large as the current rcv_mss estimate arrives with a
                // DIFFERENT length; the estimate itself follows
                // min(len, advmss)). Consumed by the windowShrinkAllowed offer
                // arithmetic and by the tcp_clamp_window growth below.
                if (payloadLength > 0) {
                    // Over-accept detection: this segment's end lies BEYOND the
                    // highest window edge ever promised (rcv_adv, still the
                    // pre-arrival value here) yet it was accepted -- the
                    // empty-queue exception. The kernel does not immediate-ACK
                    // such an arrival (rcv_neg_window); receiveSeqChanged
                    // consumes the flag and takes the delayed path.
                    if (seqGreater(tcpHeader->getSequenceNo() + payloadLength, state->rcv_adv))
                        overWindowAcceptPending = true;
                    uint32_t truesize = linuxSkbTruesize(payloadLength);
                    rcvSkbChain.push_back(std::make_pair(payloadLength, truesize));
                    rcvBufOccupancy += truesize;
                    if (payloadLength >= rcvMssEstimate) {
                        if (payloadLength != rcvMssEstimate) {
                            uint64_t ratio = ((uint64_t)payloadLength << 8) / truesize;
                            rcvScalingRatio = (uint8_t)std::min<uint64_t>(ratio ? ratio : 1, 255);
                        }
                        rcvMssEstimate = std::min<uint32_t>((uint32_t)payloadLength,
                                state->advertisedMss > 0 ? (uint32_t)state->advertisedMss : (uint32_t)payloadLength);
                    }
                    // Linux tcp_clamp_window: when the queued skbs' truesize
                    // outgrows sk_rcvbuf on a socket the application OWNS
                    // (accepted or actively opened), the buffer itself is
                    // grown toward tcp_rmem[2] rather than dropping -- an
                    // EMBRYONIC (not-yet-accepted) connection keeps its
                    // initial tcp_rmem[1] buffer untouched
                    // (ooo-before-and-after-accept pins both halves).
                    if (appOwned && !isToBeAccepted() && state->rcvBufferSize > 0
                            && rcvBufOccupancy > state->rcvBufferSize)
                    {
                        EV_DETAIL << "Receive-buffer pressure (occupancy " << rcvBufOccupancy
                                  << " > rcvbuf " << state->rcvBufferSize
                                  << "): growing sk_rcvbuf (tcp_clamp_window)\n";
                        state->rcvBufferSize = (uint32_t)std::min<uint64_t>(rcvBufOccupancy, UINT32_MAX);
                        if (state->maxRcvBuffer < state->rcvBufferSize)
                            state->maxRcvBuffer = state->rcvBufferSize;
                    }
                }

                // receiver window auto-tuning (Linux tcp_grow_window, called
                // for both in-order and out-of-order arrivals): grow the offer
                // by max(2*advmss, 2*len) toward the clamp -- a single large
                // segment can open most of the remaining room at once
                // (incr = max_t(int, incr, 2 * skb->len) in the reference).
                if (state->rcv_ssthresh > 0 && state->rcv_ssthresh < state->window_clamp && payloadLength > 0) {
                    uint32_t incr = std::max((uint32_t)(2 * state->snd_mss), (uint32_t)(2 * payloadLength));
                    uint32_t room = state->window_clamp - state->rcv_ssthresh;
                    state->rcv_ssthresh += std::min(room, incr);
                }

                if (seqGreater(state->snd_una, old_snd_una))
                    tcpAlgorithm->receivedAckForUnackedData(old_snd_una);

                // out-of-order segment?
                if (old_rcv_nxt == state->rcv_nxt) {
                    state->rcv_oooseg++;

                    emit(rcvOooSegSignal, state->rcv_oooseg);

                    // RFC 2018, page 4:
                    // "The receiver SHOULD send an ACK for every valid segment that arrives
                    // containing new data, and each of these "duplicate" ACKs SHOULD bear a
                    // SACK option."
                    if (state->sack_enabled) {
                        // store start and end sequence numbers of current oooseg in state variables
                        state->start_seqno = tcpHeader->getSequenceNo();
                        state->end_seqno = tcpHeader->getSequenceNo() + payloadLength;

                        if (old_usedRcvBuffer == receiveQueue->getAmountOfBufferedBytes()) { // D-SACK
                            state->snd_dsack = true;
                            EV_DETAIL << "SND_D-SACK SET (old_rcv_nxt == rcv_nxt duplicated oooseg rcvd)\n";
                        }
                        else { // SACK
                            state->snd_sack = true;
                            EV_DETAIL << "SND_SACK SET (old_rcv_nxt == rcv_nxt oooseg rcvd)\n";
                        }
                    }

                    tcpAlgorithm->receivedOutOfOrderSegment();
                }
                else {
                    // forward data to app
                    //
                    // FIXME observe PSH bit
                    //
                    // FIXME we should implement socket READ command, and pass up only
                    // as many bytes as requested. rcv_wnd should be decreased
                    // accordingly!
                    //
                    if (!isToBeAccepted()) {
                        sendAvailableDataToApp();
                    }
                    // if this segment "filled the gap" until the previously arrived segment
                    // that carried a FIN (i.e.rcv_nxt == rcv_fin_seq), we have to advance
                    // rcv_nxt over the FIN.
                    if (state->fin_rcvd && state->rcv_nxt == state->rcv_fin_seq) {
                        state->ack_now = true; // although not mentioned in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861] seems like we have to set ack_now
                        EV_DETAIL << "All segments arrived up to the FIN segment, advancing rcv_nxt over the FIN\n";
                        state->rcv_nxt = state->rcv_fin_seq + 1;
                        // state transitions will be done in the state machine, here we just set
                        // the proper event code (TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK)
                        event = TCP_E_RCV_FIN;

                        switch (fsm.getState()) {
                            case TCP_S_FIN_WAIT_1:
                                if (state->fin_ack_rcvd) {
                                    event = TCP_E_RCV_FIN_ACK;
                                    // start the time-wait timer, turn off the other timers
                                    cancelEvent(finWait2Timer);
                                    scheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);

                                    // we're entering TIME_WAIT, so we can signal CLOSED the user
                                    // (the only thing left to do is wait until the 2MSL timer expires)
                                }
                                break;

                            case TCP_S_FIN_WAIT_2:
                                // Start the time-wait timer, turn off the other timers.
                                cancelEvent(finWait2Timer);
                                scheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);

                                // we're entering TIME_WAIT, so we can signal CLOSED the user
                                // (the only thing left to do is wait until the 2MSL timer expires)
                                break;

                            case TCP_S_TIME_WAIT:
                                // Restart the 2 MSL time-wait timeout.
                                rescheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            else { // not enough freeRcvBuffer in rcvQueue for new segment
                state->tcpRcvQueueDrops++; // update current number of tcp receive queue drops

                emit(tcpRcvQueueDropsSignal, state->tcpRcvQueueDrops);

                // if the ACK bit is off drop the segment and return
                EV_WARN << "RcvQueueBuffer has run out, dropping segment\n";
                return TCP_E_IGNORE;
            }
        }
    }

    //
    // RFC 793: eighth, check the FIN bit,
    //
    if (tcpHeader->getFinBit()) {
        state->ack_now = true;

        //"
        // If the FIN bit is set, signal the user "connection closing" and
        // return any pending RECEIVEs with same message, advance RCV.NXT
        // over the FIN, and send an acknowledgment for the FIN.  Note that
        // FIN implies PUSH for any segment text not yet delivered to the
        // user.
        //"

        // Note: seems like RFC 793 is not entirely correct here: if the
        // segment is "above sequence" (ie. RCV.NXT < SEG.SEQ), we cannot
        // advance RCV.NXT over the FIN. Instead we remember this sequence
        // number and do it later.
        uint32_t fin_seq = (uint32_t)tcpHeader->getSequenceNo() + (uint32_t)payloadLength;

        if (state->rcv_nxt == fin_seq) {
            // advance rcv_nxt over FIN now
            EV_INFO << "FIN arrived, advancing rcv_nxt over the FIN\n";
            state->rcv_nxt++;
            // state transitions will be done in the state machine, here we just set
            // the proper event code (TCP_E_RCV_FIN or TCP_E_RCV_FIN_ACK)
            event = TCP_E_RCV_FIN;

            switch (fsm.getState()) {
                case TCP_S_FIN_WAIT_1:
                    if (state->fin_ack_rcvd) {
                        event = TCP_E_RCV_FIN_ACK;
                        // start the time-wait timer, turn off the other timers
                        cancelEvent(finWait2Timer);
                        scheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);

                        // we're entering TIME_WAIT, so we can signal CLOSED the user
                        // (the only thing left to do is wait until the 2MSL timer expires)
                    }
                    break;

                case TCP_S_FIN_WAIT_2:
                    // Start the time-wait timer, turn off the other timers.
                    cancelEvent(finWait2Timer);
                    scheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);

                    // we're entering TIME_WAIT, so we can signal CLOSED the user
                    // (the only thing left to do is wait until the 2MSL timer expires)
                    break;

                case TCP_S_TIME_WAIT:
                    // Restart the 2 MSL time-wait timeout.
                    rescheduleAfter(2 * tcpMain->getMsl(), the2MSLTimer);
                    break;

                default:
                    break;
            }
        }
        else {
            // we'll have to do it later (when an arriving segment "fills the gap")
            EV_DETAIL << "FIN segment above sequence, storing sequence number of FIN\n";
            state->fin_rcvd = true;
            state->rcv_fin_seq = fin_seq;
        }

        // TODO do PUSH stuff
    }

    if (old_rcv_nxt != state->rcv_nxt) {
        // if rcv_nxt changed, either because we received segment text or we
        // received a FIN that needs to be acked (or both), we need to send or
        // schedule an ACK.
        if (state->sack_enabled) {
            if (dupRangeFound) {
                // RFC 2883: a gap-filling (or partially duplicate) segment covered
                // data that was already buffered -- report the first duplicated
                // range as a D-SACK block; addSacks() appends the still-missing
                // out-of-order blocks (if any) after it (Linux tcp_ofo_queue ->
                // tcp_dsack_extend).
                state->start_seqno = dupStart;
                state->end_seqno = dupEnd;
                state->snd_dsack = true;
                EV_DETAIL << "SND_D-SACK SET (segment duplicates buffered range [" << dupStart << ".." << dupEnd << "))\n";
                state->ack_now = true;
            }
            else if (receiveQueue->getQueueLength() != 0) {
                // RFC 2018, page 4:
                // "If sent at all, SACK options SHOULD be included in all ACKs which do
                // not ACK the highest sequence number in the data receiver's queue."
                state->start_seqno = tcpHeader->getSequenceNo();
                state->end_seqno = tcpHeader->getSequenceNo() + payloadLength;
                state->snd_sack = true;
                EV_DETAIL << "SND_SACK SET (rcv_nxt changed, but receiveQ is not empty)\n";
                state->ack_now = true; // although not mentioned in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861] seems like we have to set ack_now
            }
        }

        // tcpAlgorithm decides when and how to do ACKs
        tcpAlgorithm->receiveSeqChanged();
    }

    if ((fsm.getState() == TCP_S_ESTABLISHED || fsm.getState() == TCP_S_SYN_RCVD) &&
        state->send_fin && state->snd_nxt == state->snd_fin_seq + 1)
    {
        // if the user issued the CLOSE command a long time ago and we've just
        // managed to send off FIN, we simulate a CLOSE command now (we had to
        // defer it at that time because we still had data in the send queue.)
        // This CLOSE will take us into the FIN_WAIT_1 state.
        EV_DETAIL << "Now we can do the CLOSE which was deferred a while ago\n";
        event = TCP_E_CLOSE;
    }

    if (fsm.getState() == TCP_S_CLOSE_WAIT && state->send_fin &&
        state->snd_nxt == state->snd_fin_seq + 1 && old_snd_nxt != state->snd_nxt)
    {
        // if we're in CLOSE_WAIT and we just got to sent our long-pending FIN,
        // we simulate a CLOSE command now (we had to defer it at that time because
        // we still had data in the send queue.) This CLOSE will take us into the
        // LAST_ACK state.
        EV_DETAIL << "Now we can do the CLOSE which was deferred a while ago\n";
        event = TCP_E_CLOSE;
    }

    return event;
}

// ----

TcpEventCode TcpConnection::processSegmentInListen(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    EV_DETAIL << "Processing segment in LISTEN\n";

    //"
    // first check for an RST
    //   An incoming RST should be ignored.  Return.
    //"
    if (tcpHeader->getRstBit()) {
        EV_INFO << "RST bit set: dropping segment\n";
        return TCP_E_IGNORE;
    }

    //"
    // second check for an ACK
    //    Any acknowledgment is bad if it arrives on a connection still in
    //    the LISTEN state.  An acceptable reset segment should be formed
    //    for any arriving ACK-bearing segment.  The RST should be
    //    formatted as follows:
    //
    //      <SEQ=SEG.ACK><CTL=RST>
    //
    //    Return.
    //"
    if (tcpHeader->getAckBit()) {
        EV_INFO << "ACK bit set: dropping segment and sending RST\n";
        sendRst(tcpHeader->getAckNo(), destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
        return TCP_E_IGNORE;
    }

    //"
    // third check for a SYN
    //"
    if (tcpHeader->getSynBit()) {
        if (tcpHeader->getFinBit()) {
            // Looks like implementations vary on how to react to SYN+FIN.
            // Some treat it as plain SYN (and reply with SYN+ACK), some send RST+ACK.
            // Let's just do the former here.
            EV_INFO << "SYN+FIN received: ignoring FIN\n";
        }

        EV_DETAIL << "SYN bit set: filling in foreign socket and sending SYN+ACK\n";

        //"
        // If the listen was not fully specified (i.e., the foreign socket was not
        // fully specified), then the unspecified fields should be filled in now.
        //"
        //
        // Also, we may need to fork, in order to leave another connection
        // LISTENing on the port. Note: forking will change our socketId.
        //
        if (state->fork) {
            TcpConnection *conn = cloneListeningConnection(); // "conn" is the clone which will handle the new connection, while "this" stay LISTENing
            tcpMain->addForkedConnection(this, conn, destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
            EV_DETAIL << "Connection forked: new connection got new socketId=" << conn->socketId << ", "
                                                                                                    "old connection keeps LISTENing with socketId=" << socketId << "\n";
            TcpEventCode forkEvent = conn->processSynInListen(tcpSegment, tcpHeader, srcAddr, destAddr);
            conn->performStateTransition(forkEvent);

            return TCP_E_IGNORE;
        }
        else {
            tcpMain->updateSockPair(this, destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
            return processSynInListen(tcpSegment, tcpHeader, srcAddr, destAddr);
        }
    }

    //"
    //  fourth other text or control
    //   So you are unlikely to get here, but if you do, drop the segment, and return.
    //"
    EV_WARN << "Unexpected segment: dropping it\n";
    return TCP_E_IGNORE;
}

TcpEventCode TcpConnection::processSynInListen(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    //"
    //  Set RCV.NXT to SEG.SEQ+1, IRS is set to SEG.SEQ and any other
    //  control or text should be queued for processing later.  ISS
    //  should be selected and a SYN segment sent of the form:
    //
    //    <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
    //
    //  SND.NXT is set to ISS+1 and SND.UNA to ISS.  The connection
    //  state should be changed to SYN-RECEIVED.
    //"
    state->rcv_nxt = tcpHeader->getSequenceNo() + 1;
    state->rcv_adv = state->rcv_nxt + state->rcv_wnd;

    emit(rcvAdvSignal, state->rcv_adv);

    state->irs = tcpHeader->getSequenceNo();
    receiveQueue->init(state->rcv_nxt); // FIXME may init twice...
    selectInitialSeqNum();

    // although not mentioned in RFC 9293, seems like we have to pick up
    // initial snd_wnd from the segment here.
    updateWndInfo(tcpHeader, true);

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) // Header options present?
        readHeaderOptions(tcpHeader);

    // Linux tcp_syncookies=2 (always-on cookies): the connection is rebuilt
    // from the cookie at the handshake ACK, and its 2-bit MSS field quantizes
    // the peer's advertised MSS DOWN to the IPv4 msstab (net/ipv4/syncookies.c
    // msstab[] = {536, 1300, 1440, 1460}: largest entry not above the
    // advertised value). wscale/SACK/TS ride the timestamp encoding and stay
    // exact (syncookies_ip4_9k pins snd_mss 1448 = 1460 - 12 against an
    // advertised 8960).
    if (tcpMain->par("syncookiesAlways").boolValue() && state->snd_mss > 536) {
        static const uint32_t msstab[] = { 536, 1300, 1440, 1460 };
        uint32_t clamped = msstab[0];
        for (uint32_t entry : msstab)
            if (entry <= state->snd_mss)
                clamped = entry;
        if (clamped < state->snd_mss) {
            EV_DETAIL << "syncookies=2: peer MSS " << state->snd_mss << " quantized to msstab " << clamped << "\n";
            state->snd_mss = clamped;
            state->snd_effmss = calculateEffectiveMss();
        }
    }

    state->ack_now = true;

    // ECN. AccECN's request codepoint (SEWA: ECE=CWR=AE=1) is checked first -- it's a
    // superset of the classic-ECN-willing bit pattern (ECE=CWR=1), so without this check
    // first an AccECN SYN would also satisfy the classic branch below and get misread as a
    // plain RFC 3168 request.
    if (tcpHeader->getEceBit() && tcpHeader->getCwrBit() && tcpHeader->getAeBit()
        && (state->ecnMode == TCP_ECN_MODE_ACCECN || state->ecnMode == TCP_ECN_MODE_ACCECN_PASSIVE))
    {
        state->endPointIsWillingECN = true;
        state->accEcnNegotiated = true;
        EV << "AccECN-setup SYN received\n";
    }
    else if (tcpHeader->getEceBit() == true && tcpHeader->getCwrBit() == true
             && (!tcpHeader->getAeBit() || state->ecnMode == TCP_ECN_MODE_RFC3168)) {
        // Classic branch. An RFC3168-mode host ACCEPTS an AE-carrying SYN as
        // ECN-willing -- Linux tcp_ecn_create_request's condition
        // (!ect || th->res1 || th->ae) && ecn_ok treats the AE bit as evidence
        // FOR a compliant peer (RFC 8311 section 4.3 allows future extensions
        // on that bit), so an AccECN SYN falls back to plain RFC 3168 here
        // (accecn_to_rfc3168 pins the SE. SYN-ACK and ect0-marked data). A
        // PASSIVE-mode host still requires AE=0: it never volunteered for ECN
        // and must not read a foreign bit pattern as a classic request.
        state->endPointIsWillingECN = true;
        EV << "ECN-setup SYN packet received\n";
    }

    // TCP Fast Open (RFC 7413): a validated-cookie SYN carrying data is accepted and
    // delivered to the app now, ahead of the 3WHS completing -- readHeaderOptions()
    // above has already run processFastOpenOption() and populated
    // state->fastopenCookie*. Advancing rcv_nxt here (before sendSynAck() below)
    // makes the SYN-ACK's ack number naturally cover the SYN and the data in one
    // segment, with no change needed to sendSynAck() itself.
    bool tfoAttempted = state->fastopenCookieRequested || state->fastopenCookieValid || state->fastopenSendCookieOption;
    // Cookie-less mode (RFC 7413 section 4.1.3, sysctl TFO_SERVER_COOKIE_NOT_REQD):
    // the listener accepts the SYN data even when the SYN carries no cookie option
    // at all, so fastopenCookieValid (and tfoAttempted) stay false here.
    if (state->fastopenServerEnabled && (state->fastopenCookieValid || state->fastopenAcceptWithoutCookie)) {
        // The TFO acceptance itself is payload-independent: a zero-payload SYN
        // with a valid cookie still creates a fully-accelerated connection
        // whose app may respond from SYN_RCVD (basic-zero-payload scripts).
        state->fastopenAccelerated = true;
        B payloadLen = B(tcpSegment->getByteLength()) - tcpHeader->getHeaderLength();
        if (payloadLen > B(0) && hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) {
            updateRcvQueueVars();
            // insertBytesFromSegment() indexes the payload by tcpHeader's own
            // SequenceNo, which is correct for a plain data segment but is the SYN's
            // OWN sequence number here (RFC 793: SYN consumes one sequence number, so
            // the data actually starts at SEG.SEQ+1) -- pass a sequence-shifted copy
            // of the header so the receive queue doesn't mistake the first data byte
            // for a 1-byte overlap with the already-initialized rcv_nxt (=IRS+1) and
            // silently drop it.
            auto synShiftedHeader = staticPtrCast<TcpHeader>(tcpHeader->dupShared());
            synShiftedHeader->setSequenceNo(tcpHeader->getSequenceNo() + 1);
            state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, synShiftedHeader);
            updateRcvQueueVars();
            sendAvailableDataToApp(); // deliver before the 3WHS completes -- the RFC 7413 win
            state->fastopenSynDataAccepted = true; // surfaced as TCPI_OPT_SYN_DATA in tcp_info
            EV_INFO << "Fast Open: " << (state->fastopenCookieValid ? "cookie valid" : "no cookie required")
                    << ", accepting " << payloadLen
                    << " bytes of SYN data before handshake completion\n";
        }
    }

    sendSynAck();
    startSynRexmitTimer();

    if (!connEstabTimer->isScheduled())
        scheduleAfter(TCP_TIMEOUT_CONN_ESTAB, connEstabTimer);

    //"
    // Note that any other incoming control or data (combined with SYN)
    // will be processed in the SYN-RECEIVED state, but processing of SYN
    // and ACK should not be repeated.
    //"
    // We don't send text in SYN or SYN+ACK, but accept it. Otherwise
    // there isn't much left to do: RST, SYN, ACK, FIN got processed already,
    // so there's only URG and PSH left to handle.
    //
    // Also skipped when the Fast Open block above already consumed the payload
    // (fastopenSynDataAccepted): in the cookie-less mode tfoAttempted is false
    // (no cookie option was present), and re-inserting here with the UNSHIFTED
    // sequence number -- after rcv_nxt has already advanced past the whole
    // payload -- would compute a negative remainder and crash in peekDataAt()
    // ("offset is out of range").
    if (!tfoAttempted && !state->fastopenSynDataAccepted
            && B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength()) {
        updateRcvQueueVars();

        if (hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) { // enough freeRcvBuffer in rcvQueue for new segment?
            receiveQueue->insertBytesFromSegment(tcpSegment, tcpHeader);
        }
        else { // not enough freeRcvBuffer in rcvQueue for new segment
            state->tcpRcvQueueDrops++; // update current number of tcp receive queue drops

            emit(tcpRcvQueueDropsSignal, state->tcpRcvQueueDrops);

            EV_WARN << "RcvQueueBuffer has run out, dropping segment\n";
            return TCP_E_IGNORE;
        }
    }

    if (tcpHeader->getUrgBit() || tcpHeader->getPshBit())
        EV_DETAIL << "Ignoring URG and PSH bits in SYN\n"; // TODO

    return TCP_E_RCV_SYN; // this will take us to SYN_RCVD
}

TcpEventCode TcpConnection::processSegmentInSynSent(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader, L3Address srcAddr, L3Address destAddr)
{
    EV_DETAIL << "Processing segment in SYN_SENT\n";

    //"
    // first check the ACK bit
    //
    //   If the ACK bit is set
    //
    //     If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset (unless
    //     the RST bit is set, if so drop the segment and return)
    //
    //       <SEQ=SEG.ACK><CTL=RST>
    //
    //     and discard the segment.  Return.
    //
    //     If SND.UNA =< SEG.ACK =< SND.NXT then the ACK is acceptable.
    //"
    if (tcpHeader->getAckBit()) {
        if (seqLE(tcpHeader->getAckNo(), state->iss) || seqGreater(tcpHeader->getAckNo(), state->snd_nxt)) {
            if (tcpHeader->getRstBit()) {
                EV_DETAIL << "ACK+RST bit set but wrong AckNo, ignored\n";
                // TCP Fast Open active blackhole detection: an out-of-order RST
                // (wrong AckNo) while a data-carrying TFO SYN is still outstanding
                // is a classic middlebox-interference symptom -- some boxes that
                // don't understand the TFO option strip it and desync sequence
                // numbers, producing a stray RST like this one.
                if (state->fastopenSynDataLen > 0)
                    tcpMain->recordFastOpenBlackhole();
            }
            else {
                EV_DETAIL << "ACK bit set but wrong AckNo, sending RST\n";
                sendRst(tcpHeader->getAckNo(), destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());
            }
            return TCP_E_IGNORE;
        }

        EV_DETAIL << "ACK bit set, AckNo acceptable\n";
    }

    //"
    // second check the RST bit
    //
    //   If the RST bit is set
    //
    //     If the ACK was acceptable then signal the user "error:
    //     connection reset", drop the segment, enter CLOSED state,
    //     delete TCB, and return.  Otherwise (no ACK) drop the segment
    //     and return.
    //"
    if (tcpHeader->getRstBit()) {
        if (tcpHeader->getAckBit()) {
            EV_DETAIL << "RST+ACK: performing connection reset\n";
            sendIndicationToApp(TCP_I_CONNECTION_RESET);

            return TCP_E_RCV_RST;
        }
        else {
            EV_DETAIL << "RST without ACK: dropping segment\n";

            return TCP_E_IGNORE;
        }
    }

    //"
    // third check the security and precedence -- not done
    //
    // fourth check the SYN bit
    //
    //   This step should be reached only if the ACK is ok, or there is
    //   no ACK, and it the segment did not contain a RST.
    //
    //   If the SYN bit is on and the security/compartment and precedence
    //   are acceptable then,
    //"
    if (tcpHeader->getSynBit()) {
        //
        //   RCV.NXT is set to SEG.SEQ+1, IRS is set to
        //   SEG.SEQ.  SND.UNA should be advanced to equal SEG.ACK (if there
        //   is an ACK), and any segments on the retransmission queue which
        //   are thereby acknowledged should be removed.
        //
        state->rcv_nxt = tcpHeader->getSequenceNo() + 1;
        state->rcv_adv = state->rcv_nxt + state->rcv_wnd;

        emit(rcvAdvSignal, state->rcv_adv);

        state->irs = tcpHeader->getSequenceNo();
        receiveQueue->init(state->rcv_nxt);

        if (tcpHeader->getAckBit()) {
            state->snd_una = tcpHeader->getAckNo();
            sendQueue->discardUpTo(state->snd_una);

            if (state->sack_enabled)
                rexmitQueue->discardUpTo(state->snd_una);

            // although not mentioned in RFC 793, seems like we have to pick up
            // initial snd_wnd from the segment here.
            updateWndInfo(tcpHeader, true);
        }

        // this also seems to be a good time to learn our local IP address
        // (was probably unspecified at connection open)
        tcpMain->updateSockPair(this, destAddr, srcAddr, tcpHeader->getDestPort(), tcpHeader->getSrcPort());

        //"
        //   If SND.UNA > ISS (our SYN has been ACKed), change the connection
        //   state to ESTABLISHED, form an ACK segment
        //
        //     <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
        //
        //   and send it.  Data or controls which were queued for
        //   transmission may be included.  If there are other controls or
        //   text in the segment then continue processing at the sixth step
        //   below where the URG bit is checked, otherwise return.
        //"
        if (seqGreater(state->snd_una, state->iss)) {
            EV_INFO << "SYN+ACK bits set, connection established.\n";

            // TCP Fast Open client (RFC 7413): the SYN carried data and the
            // SYN-ACK acked ALL of it -- surface as TCPI_OPT_SYN_DATA, the
            // same tcp_info bit the server side sets. Linux tp->syn_data_acked
            // is true only when nothing of the SYN data remains unacked
            // (tcp_rcv_fastopen_synack: syn_data && !data); a partial ack
            // leaves it clear (syn-data-partial-or-over-ack: 9 of 18 bytes).
            if (state->fastopenSynDataLen > 0
                && seqGE(state->snd_una, state->iss + 1 + state->fastopenSynDataLen))
                state->fastopenSynDataAccepted = true;


            // RFC says "continue processing at the sixth step below where
            // the URG bit is checked". Those steps deal with: URG, segment text
            // (and PSH), and FIN.
            // Now: URG and PSH we don't support yet; in SYN+FIN we ignore FIN;
            // with segment text we just take it easy and put it in the receiveQueue
            // -- we'll forward it to the user when more data arrives.
            bool synAckFin = tcpHeader->getFinBit();

            if (B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength()) {
                updateRcvQueueVars();

                if (hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) { // enough freeRcvBuffer in rcvQueue for new segment?
                    // advance rcv_nxt over the SYN-ACK's data (RFC 793 permits data on
                    // SYN-ACK; a TFO server may respond in the same segment), so that
                    // the handshake ACK acknowledges it. The SYN consumes one
                    // sequence number, so index the payload from SEG.SEQ+1 via a
                    // sequence-shifted header copy (same pattern as the TFO server's
                    // SYN-data acceptance in processSynInListen()).
                    auto synShiftedHeader = staticPtrCast<TcpHeader>(tcpHeader->dupShared());
                    synShiftedHeader->setSequenceNo(tcpHeader->getSequenceNo() + 1);
                    state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, synShiftedHeader);
                }
                else { // not enough freeRcvBuffer in rcvQueue for new segment
                    state->tcpRcvQueueDrops++; // update current number of tcp receive queue drops

                    emit(tcpRcvQueueDropsSignal, state->tcpRcvQueueDrops);

                    EV_WARN << "RcvQueueBuffer has run out, dropping segment\n";
                    return TCP_E_IGNORE;
                }
            }

            if (tcpHeader->getUrgBit() || tcpHeader->getPshBit())
                EV_DETAIL << "Ignoring URG and PSH bits in SYN+ACK\n"; // TODO

            if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) // Header options present?
                readHeaderOptions(tcpHeader);

            // Linux tcp_rcv_fastopen_synack(): a TFO connection's SYN-ACK
            // refreshes the cached peer MSS EVERY time (the cookie only when
            // one is present) -- a later cookie-less SYN-ACK advertising a
            // larger MSS restores the cache after an earlier small-MSS server
            // shrank it (syn-data-only-syn-acked's 1040 -> 1460 -> 1420-cap
            // sequence pins this). Cache the RAW advertised value, not
            // snd_mss: a local TCP_MAXSEG clamp on THIS connection must not
            // shrink the metrics cache (the kernel reparses the SYN-ACK to
            // bypass the user clamp; syn-data-mss pins the next SYN's
            // 1300-byte payload from an advertised 1340 despite this
            // connection's TCP_MAXSEG 1040).
            if (state->fastopenRequested)
                tcpMain->updateFastOpenCachedMss(remoteAddr,
                        state->peerAdvertisedMss > 0 ? state->peerAdvertisedMss : state->snd_mss);

            // RFC 7323 / Linux tcp_rcv_synsent_state_process (PAWSACTIVEREJECTED):
            // a SYN-ACK whose TSecr does not echo anything this connection could
            // have sent (it must lie between the SYN's send time and now on our
            // timestamp clock) is repelled with <SEQ=SEG.ACK><CTL=RST> and the
            // segment is dropped -- the connection stays in SYN_SENT awaiting a
            // valid SYN-ACK (synack-data TEST5's deliberate bad-ecr probe).
            if (state->rcv_initial_ts && state->lastRcvdTSecr != 0 && state->handshakeSentTime >= SIMTIME_ZERO) {
                uint32_t tsLow = convertSimtimeToTS(state->handshakeSentTime);
                uint32_t tsHigh = convertSimtimeToTS(simTime());
                if (seqLess(state->lastRcvdTSecr, tsLow) || seqGreater(state->lastRcvdTSecr, tsHigh)) {
                    EV_WARN << "SYN-ACK TSecr " << state->lastRcvdTSecr << " outside [" << tsLow << ", "
                            << tsHigh << "] -- repelling with RST (PAWSACTIVEREJECTED)\n";
                    sendRst(tcpHeader->getAckNo());
                    return TCP_E_IGNORE;
                }
            }

            // notify tcpAlgorithm (it has to send ACK of SYN) and app layer
            state->ack_now = true;
            state->snd_effmss = calculateEffectiveMss();

            // ECN. Resolved BEFORE tcpAlgorithm->established(true) below: that call
            // synchronously sends the connection-completing 3rd ACK, and AccECN needs that
            // ACK's ACE field to reflect the just-negotiated state (matching the kernel's
            // handling of the analogous 3rd-ACK case) -- ordering matters here in a way it
            // never did for classic ECN, which doesn't touch this particular ACK's flags.
            if (state->aeSynSent) {
                // draft-ietf-tcpm-accurate-ecn 3WHS: decode the SYN-ACK's (ECE,CWR,AE) triple
                // in response to our SEWA (AccECN-requesting) SYN.
                // Table 2 of RFC 9768 / Linux tcp_ecn_rcv_synack: the ACE value
                // (AE<<2 | CWR<<1 | ECE) of the SYN-ACK decides the mode.
                // 0b000 and 0b111 = no ECN; 0b001 = peer speaks only classic
                // ECN, fall back; EVERY other value (0b010..0b110) = AccECN
                // accepted, the value additionally encoding how our SYN
                // arrived (serverside_accecn_disabled1 pins 0b101 as accept).
                uint8_t synAckAce = (uint8_t)((tcpHeader->getAeBit() ? 4 : 0)
                        | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0));
                if (synAckAce == 0 || synAckAce == 7) {
                    state->ect = false;
                    EV << "AccECN request received a non-ECN-setup SYN-ACK... ECN is disabled.\n";
                }
                else if (synAckAce == 1) {
                    state->ecnMode = TCP_ECN_MODE_RFC3168;
                    state->ect = true;
                    EV << "AccECN request received classic-ECN SYN-ACK... falling back to RFC 3168 ECN.\n";
                }
                else {
                    state->accEcnNegotiated = true;
                    state->ect = true;
                    EV << "AccECN-setup SYN-ACK received (ACE=" << (int)synAckAce << ")... AccECN is enabled.\n";
                }
                state->aeSynSent = false;
            }
            else if (state->ecnSynSent) {
                if (tcpHeader->getEceBit() && !tcpHeader->getCwrBit()) {
                    state->ect = true;
                    EV << "ECN-setup SYN-ACK packet was received... ECN is enabled.\n";
                }
                else {
                    state->ect = false;
                    EV << "non-ECN-setup SYN-ACK packet was received... ECN is disabled.\n";
                }
                state->ecnSynSent = false;
            }
            else {
                state->ect = false;
                if (tcpHeader->getEceBit() && !tcpHeader->getCwrBit())
                    EV << "ECN-setup SYN-ACK packet was received... ECN is disabled.\n";
            }

            // Seed the RTT estimator from the handshake RTT (Linux measures the
            // SYN<->SYN-ACK exchange via tcp_ack_update_rtt/tcp_synack_rtt_meas and
            // enters ESTABLISHED with srtt/rttvar -- and hence the first RTO and any
            // TLP probe timeout -- already RTT-scaled instead of the 1 s initial
            // default). Karn: skipped if our handshake segment was retransmitted.
            if (state->syn_rexmit_count == 0 && state->handshakeSentTime >= SIMTIME_ZERO)
                tcpAlgorithm->rttMeasurementComplete(state->handshakeSentTime, simTime());

            // RFC 7413 section 4.1: SYN data the SYN-ACK did NOT acknowledge
            // (server acked only the SYN, or a partial range) is retransmitted
            // immediately on connection establishment -- Linux does this from
            // tcp_rcv_synsent_state_process, and the corpus pins the retransmit
            // as the FIRST post-handshake segment with the handshake ACK
            // piggybacked on it ("> P. 1:1001(1000) ack 1", no separate bare
            // ACK). Pull snd_nxt back to the unacked point so established()'s
            // send-data-with-first-ACK path emits exactly that.
            bool fastopenSynDataRexmit = state->fastopenSynDataLen > 0 && seqLess(state->snd_una, state->snd_max);
            if (fastopenSynDataRexmit) {
                EV_INFO << "Fast Open: SYN data [" << state->snd_una << ", " << state->snd_max
                        << ") not acknowledged by the SYN-ACK, retransmitting with the handshake ACK\n";
                state->snd_nxt = state->snd_una;
                // afterRto is the sanctioned "snd_nxt deliberately pulled back"
                // signal: without it sendData() immediately resets snd_nxt to
                // snd_max and nothing is retransmitted. It auto-clears once
                // snd_nxt catches back up to snd_max.
                state->afterRto = true;
                // The unacked SYN data no longer counts as in flight (Linux's
                // fallback requeues the skb as plain pending data, packets_out
                // drops to 0): without the lost mark the pipe still carries it
                // and, with the post-SYN-rexmit IW of one segment, allowedToSend
                // collapses to zero and only a bare handshake ACK leaves
                // (cookie-less-sendto's non-blocking test).
                if (state->sack_enabled && rexmitQueue->getQueueLength() > 0)
                    rexmitQueue->markLost(state->snd_una, state->snd_max);
                // No Nagle/PSH special-casing needed anymore: Minshall's check
                // never holds this partial (no unacked SMALL segment is in
                // flight), and the PSH comes from the push boundary sendSyn()
                // recorded at the SYN-data end (Linux marks the syn_data skb
                // with TCPHDR_PSH at creation).
            }

            // RFC 793 permits FIN on the SYN-ACK; Linux processes it and lands
            // in CLOSE_WAIT (tcp_fin) -- fastopen/client/synack-data's TEST2
            // expects the single handshake ACK to cover data AND FIN (ack 1402).
            // Advance rcv_nxt over the FIN BEFORE established(true) sends that
            // ACK; the state hop goes SYN_SENT -> ESTABLISHED here, then the
            // returned RCV_FIN takes ESTABLISHED -> CLOSE_WAIT in the caller
            // (stateEntered defers TCP_I_PEER_CLOSED until the data is read,
            // like a normal FIN behind undelivered data).
            if (synAckFin) {
                EV_INFO << "FIN on the SYN-ACK: advancing rcv_nxt over the FIN, will enter CLOSE_WAIT\n";
                state->fin_rcvd = true;
                state->rcv_fin_seq = state->rcv_nxt;
                state->rcv_nxt = state->rcv_fin_seq + 1;
            }

            // notify tcpAlgorithm (it has to send ACK of SYN) and app layer
            state->ack_now = true;
            tcpAlgorithm->established(true);
            tcpMain->emit(Tcp::tcpConnectionAddedSignal, this);
            sendEstabIndicationToApp();
            // deliver any data that rode the SYN-ACK (inserted above) -- the
            // normal per-segment delivery only runs in the data-transfer
            // states, so without this the app would never see these bytes
            // (and a poll() right after the handshake would miss POLLIN)
            sendAvailableDataToApp();

            if (synAckFin) {
                performStateTransition(TCP_E_RCV_SYN_ACK); // SYN_SENT -> ESTABLISHED now...
                return TCP_E_RCV_FIN;                      // ...so the caller's transition lands in CLOSE_WAIT
            }

            // This will trigger transition to ESTABLISHED. Timers and notifying
            // app will be taken care of in stateEntered().
            return TCP_E_RCV_SYN_ACK;
        }

        //"
        //   Otherwise enter SYN-RECEIVED, form a SYN,ACK segment
        //
        //     <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
        //
        //   and send it.  If there are other controls or text in the
        //   segment, queue them for processing after the ESTABLISHED state
        //   has been reached, return.
        //"
        EV_INFO << "SYN bit set: sending SYN+ACK\n";
        // Simultaneous open: consume the crossing SYN's options BEFORE building
        // our SYN-ACK (mirrors processSegmentInListen()) -- otherwise
        // rcv_sack_perm/ws/ts stay unset and the reply advertises nothing.
        if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH)
            readHeaderOptions(tcpHeader);
        state->snd_max = state->snd_nxt = state->iss;
        emit(sndMaxSignal, state->snd_max);
        sendSynAck();
        startSynRexmitTimer();
        // TFO simultaneous open: our SYN's data stays queued and OUTSTANDING.
        // Linux keeps snd_nxt spanning it (the SYN-ACK reuses the SYN skb's
        // sequence, nothing is rewound), so the peer's eventual SYN-ACK acking
        // the data (ack 1001) is acceptable and the dup-segment ACK goes out
        // with SEQ = 1001, not a SYN-ACK retransmit (simultaneous-fast-open).
        if (state->fastopenSynDataLen > 0) {
            state->snd_max = state->snd_nxt = state->iss + 1 + state->fastopenSynDataLen;
            emit(sndMaxSignal, state->snd_max);
        }

        // Note: code below is similar to processing SYN in LISTEN.

        // For consistency with that code, we ignore SYN+FIN here
        if (tcpHeader->getFinBit())
            EV_DETAIL << "SYN+FIN received: ignoring FIN\n";

        // Data on the crossing SYN is DISCARDED: Linux tcp_rcv_synsent_state_
        // process moves to SYN_RECV and drops the segment -- a client socket
        // has no TFO server context to accept SYN data into, and the golden
        // pins the peer retransmitting it after the handshake
        // (simultaneous-fast-open: "The other end retries").
        if (B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength())
            EV_DETAIL << "Discarding data on the crossing SYN (simultaneous open)\n";

        if (tcpHeader->getUrgBit() || tcpHeader->getPshBit())
            EV_DETAIL << "Ignoring URG and PSH bits in SYN\n"; // TODO

        return TCP_E_RCV_SYN;
    }

    //"
    // fifth, if neither of the SYN or RST bits is set then drop the
    // segment and return.
    //"
    return TCP_E_IGNORE;
}

TcpEventCode TcpConnection::processRstInSynReceived(const Ptr<const TcpHeader>& tcpHeader)
{
    EV_DETAIL << "Processing RST in SYN_RCVD\n";

    //"
    // If this connection was initiated with a passive OPEN (i.e.,
    // came from the LISTEN state), then return this connection to
    // LISTEN state and return.  The user need not be informed.  If
    // this connection was initiated with an active OPEN (i.e., came
    // from SYN-SENT state) then the connection was refused, signal
    // the user "connection refused".  In either case, all segments
    // on the retransmission queue should be removed.  And in the
    // active OPEN case, enter the CLOSED state and delete the TCB,
    // and return.
    //"

    sendQueue->discardUpTo(sendQueue->getBufferEndSeq()); // flush send queue

    rexmitQueue->discardUpTo(rexmitQueue->getBufferEndSeq()); // flush rexmit queue

    if (state->active) {
        // signal "connection refused"
        sendIndicationToApp(TCP_I_CONNECTION_REFUSED);
    }

    // on RCV_RST the FSM goes to CLOSED for an active open, a FORKED connection
    // (a Linux child socket must die -- re-listening would duplicate the still-
    // existing listener) or a TFO-accelerated connection (app-visible state
    // exists); only a plain non-forked passive open returns to LISTEN (RFC 793).
    return TCP_E_RCV_RST;
}

bool TcpConnection::processAckInEstabEtc(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    EV_DETAIL << "Processing ACK in a data transfer state\n";

    int payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();

    // ECN. AccECN connections repurpose eceBit as part of the post-handshake ACE counter
    // (decoded separately below, near the end of this function) -- classic ECE-echo
    // consumption must not also read it here, or congestion control (TcpReno/TcpCubic/DcTcp,
    // all gated on gotEce) would spuriously react to ACE bit-pattern noise instead of a real
    // congestion signal.
    TcpStateVariables *state = getStateForUpdate();
    if (state && state->ect && !state->accEcnNegotiated) {
        if (tcpHeader->getEceBit() == true)
            EV_INFO << "Received packet with ECE\n";

        state->gotEce = tcpHeader->getEceBit();
    }

    //
    //"
    //  If SND.UNA < SEG.ACK =< SND.NXT then, set SND.UNA <- SEG.ACK.
    //  Any segments on the retransmission queue which are thereby
    //  entirely acknowledged are removed.  Users should receive
    //  positive acknowledgments for buffers which have been SENT and
    //  fully acknowledged (i.e., SEND buffer should be returned with
    //  "ok" response).  If the ACK is a duplicate
    //  (SEG.ACK < SND.UNA), it can be ignored.  If the ACK acks
    //  something not yet sent (SEG.ACK > SND.NXT) then send an ACK,
    //  drop the segment, and return.
    //
    //  If SND.UNA < SEG.ACK =< SND.NXT, the send window should be
    //  updated.  If (SND.WL1 < SEG.SEQ or (SND.WL1 = SEG.SEQ and
    //  SND.WL2 =< SEG.ACK)), set SND.WND <- SEG.WND, set
    //  SND.WL1 <- SEG.SEQ, and set SND.WL2 <- SEG.ACK.
    //
    //  Note that SND.WND is an offset from SND.UNA, that SND.WL1
    //  records the sequence number of the last segment used to update
    //  SND.WND, and that SND.WL2 records the acknowledgment number of
    //  the last segment used to update SND.WND.  The check here
    //  prevents using old segments to update the window.
    //"
    // Note: should use SND.MAX instead of SND.NXT in above checks
    //
    if (seqGE(state->snd_una, tcpHeader->getAckNo())) {
        //
        // duplicate ACK? A received TCP segment is a duplicate ACK if all of
        // the following apply:
        //    (1) snd_una == ackNo
        //    (2) segment contains no data
        //    (3) there's unacked data (snd_una != snd_max)
        //
        // Note: ssfnet uses additional constraint "window is the same as last
        // received (not an update)" -- we don't do that because window updates
        // are ignored anyway if neither seqNo nor ackNo has changed.
        //
        if (state->snd_una == tcpHeader->getAckNo() && payloadLength == 0 && state->snd_una != state->snd_max) {
            state->dupacks++;

            emit(dupAcksSignal, state->dupacks);

            // we need to update send window even if the ACK is a dupACK, because rcv win
            // could have been changed if faulty data receiver is not respecting the "do not shrink window" rule
            updateWndInfo(tcpHeader);

            tcpAlgorithm->receivedDuplicateAck();
        }
        else {
            // if doesn't qualify as duplicate ACK, just ignore it.
            if (payloadLength == 0) {
                if (state->snd_una != tcpHeader->getAckNo())
                    EV_DETAIL << "Old ACK: ackNo < snd_una\n";
                else if (state->snd_una == state->snd_max)
                    EV_DETAIL << "ACK looks duplicate but we have currently no unacked data (snd_una == snd_max)\n";
            }

            // reset counter
            state->dupacks = 0;

            emit(dupAcksSignal, state->dupacks);
        }
    }
    else if (seqLE(tcpHeader->getAckNo(), state->snd_max)) {
        // ack in window.
        uint32_t old_snd_una = state->snd_una;
        state->snd_una = tcpHeader->getAckNo();

        emit(unackedSignal, state->snd_max - state->snd_una);

        // after retransmitting a lost segment, we may get an ack well ahead of snd_nxt
        if (seqLess(state->snd_nxt, state->snd_una))
            state->snd_nxt = state->snd_una;

        // RFC 1323, page 36:
        // "If SND.UNA < SEG.ACK =< SND.NXT then, set SND.UNA <- SEG.ACK.
        // Also compute a new estimate of round-trip time.  If Snd.TS.OK
        // bit is on, use my.TSclock - SEG.TSecr; otherwise use the
        // elapsed time since the first segment in the retransmission
        // queue was sent.  Any segments on the retransmission queue
        // which are thereby entirely acknowledged."
        if (state->ts_enabled)
            tcpAlgorithm->rttMeasurementCompleteUsingTS(getTSecr(tcpHeader));
        // Note: If TS is disabled the RTT measurement is completed in TcpBaseAlg::receivedDataAck()

        uint32_t discardUpToSeq = state->snd_una;

        // our FIN acked?
        if (state->send_fin && tcpHeader->getAckNo() == state->snd_fin_seq + 1) {
            // set flag that our FIN has been acked
            EV_DETAIL << "ACK acks our FIN\n";
            state->fin_ack_rcvd = true;
            discardUpToSeq--; // the FIN sequence number is not real data
        }

        // Notify the algorithm while the scoreboard for the acked range is still
        // valid (i.e. before it is discarded below): transmit counts and SACK state
        // for [old_snd_una, discardUpToSeq) are what lets a recovery algorithm tell
        // reordering apart from loss.
        tcpAlgorithm->segmentsAcked(old_snd_una, discardUpToSeq);

        // acked data no longer needed in send queue
        sendQueue->discardUpTo(discardUpToSeq);

        // TCP_INFO trio (busy_time): read-only bookkeeping -- if this ACK just
        // caught snd_una up to snd_max with nothing left queued either, the
        // connection has gone fully idle. See enqueueSendCommandData() for the
        // matching "became busy" entry.
        if (state->busyStartTime >= SIMTIME_ZERO && state->snd_una == state->snd_max
            && sendQueue->getBytesAvailable(state->snd_nxt) == 0)
        {
            state->busyTimeAccumulated += simTime() - state->busyStartTime;
            state->busyStartTime = -1;
        }

        // acked data no longer needed in rexmit queue
        rexmitQueue->discardUpTo(discardUpToSeq);

        // A plain cumulative ACK carries no SACK option, so processSACKOption()
        // does not run to recompute the SACK scoreboard byte count. Refresh it
        // after the discard so tcpi_sacked reflects only what is still SACKed
        // above snd_una (Linux tp->sacked_out drops as snd_una catches up); a full
        // ACK that ends recovery must report 0, not the stale pre-ACK count. The
        // next SACK's delivered-delta baseline (sackedBytes_old) is re-taken from
        // this value in processSACKOption(), so the PRR accounting stays consistent.
        if (state->sack_enabled)
            state->sackedBytes = rexmitQueue->getTotalAmountOfSackedBytes();

        updateWndInfo(tcpHeader);

        // if segment contains data, wait until data has been forwarded to app before sending ACK,
        // otherwise we would use an old ACKNo
        //
        // The handshake-completing ACK (fsm still SYN_RCVD here) is normally
        // excluded: it only acks the SYN, and the algorithm was just
        // initialized by established() above. But a TCP Fast Open server may
        // have sent response DATA from SYN_RCVD -- when the handshake ACK
        // also acks beyond the SYN-ACK's sequence slot (iss+1), it is a data
        // ack and must run the algorithm's ack processing, or the data's
        // REXMIT/probe timers stay armed after everything is acked.
        bool acksFastOpenData = state->fastopenSynDataAccepted && seqGreater(tcpHeader->getAckNo(), state->iss + 1);
        if (payloadLength == 0 && (fsm.getState() != TCP_S_SYN_RCVD || acksFastOpenData))
            tcpAlgorithm->receivedAckForUnackedData(old_snd_una);
    }
    else {
        ASSERT(seqGreater(tcpHeader->getAckNo(), state->snd_max)); // from if-ladder

        // send an ACK, drop the segment, and return.
        tcpAlgorithm->receivedAckForUnsentData(tcpHeader->getAckNo());

        return false; // means "drop"
    }

    // AccECN: ACE field read side -- mod-8 delta resolution
    // (design reference: tcp_accecn_process/__tcp_accecn_process, tcp_input.c, cited for the
    // naive-delta + safeDelta shape only, reimplemented against INET's own byte-oriented
    // state). Skipped on the handshake-completing ACK (fsm still SYN_RCVD here, i.e. the
    // very first ACE value this side has ever seen from the peer) -- there's no prior
    // baseline to diff against yet.
    //
    // Forward-progress guard: __tcp_accecn_process returns 0 up front unless the
    // ACK makes forward progress (FLAG_FORWARD_PROGRESS | FLAG_TS_PROGRESS). An
    // ACK that acks no new data (snd_una did not advance, so deliveredBytes is
    // unchanged since this segment's prrDeliveredMark snapshot) is a pure /
    // duplicate ACK and must not move the CE counters -- neither the ACE-field
    // delta nor the AccECN option's CEB delta. readHeaderOptions() left the
    // option baseline (peerReportedCeBytes) unadvanced, so any accumulated delta
    // is instead consumed by the next forward-progress ACK, exactly as Linux
    // defers it. TS-only progress (FLAG_TS_PROGRESS, a positive ts_recent delta
    // recorded per-segment as accEcnTsProgress) also qualifies: an ACK that acks
    // no new data but carries a FRESH timestamp is not a reordered duplicate, so
    // its ACE value is trustworthy (accecn tsprogress/tsnoprogress pin both sides
    // of this: fresh TSval counts the fake CE, a stale TSval must not).
    // AccECN third-ack ACE handling (RFC 9768 Table 4 / Linux tcp_accecn_third_ack,
    // called from tcp_ecn_openreq_child): on the handshake-completing ACK the ACE
    // field echoes how the SYN-ACK arrived (Table 3 handshake encoding, not yet a
    // counter). 0b110 = "SYN-ACK was delivered CE-marked" seeds delivered_ce to 1
    // -- which also aligns the mod-8 baseline, since the peer's own ACE counter
    // started counting from that CE. Only a data-less ACK is validated, like Linux.
    // (Linux additionally validates the claimed ECN field against what the SYN-ACK
    // was sent with unless net.ipv4.tcp_ecn_fallback=0; the pinning script,
    // accecn synack_ce_updates_delivered_ce, runs with fallback disabled.)
    if (state->accEcnNegotiated && fsm.getState() == TCP_S_SYN_RCVD && payloadLength == 0) {
        uint8_t handshakeAce = (uint8_t)((tcpHeader->getAeBit() ? 4 : 0)
                | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0));
        if (handshakeAce == 6 && state->deliveredCePkts == 0) {
            state->deliveredCePkts = 1;
            emit(deliveredCeSignal, (unsigned long)state->deliveredCePkts);
            EV_INFO << "AccECN third ACK: ACE=0b110, SYN-ACK was CE-marked -- delivered_ce seeded to 1\n";
        }
        else if (handshakeAce == 0 && state->ect) {
            // Table 4 case 0x0: an ALL-ZERO ACE on the third ACK is invalid --
            // a middlebox bleached the handshake feedback (Linux sets
            // TCP_ACCECN_ACE_FAIL_RECV). Stop marking ECT; the ACE/option
            // feedback machinery keeps running (negotiation_bleach pins
            // [noecn] data segments that still carry ACE flags + the option).
            EV_INFO << "AccECN third ACK: ACE=0b000 (bleached) -- disabling ECT marking\n";
            state->ect = false;
        }
    }

    if (state->accEcnNegotiated && fsm.getState() != TCP_S_SYN_RCVD
            && (state->deliveredBytes != state->prrDeliveredMark || state->accEcnTsProgress)) {
        bool ae = tcpHeader->getAeBit();
        bool cwr = tcpHeader->getCwrBit();
        bool ece = tcpHeader->getEceBit();
        uint8_t receivedAce = (uint8_t)((ae ? 4 : 0) | (cwr ? 2 : 0) | (ece ? 1 : 0));

        // deliveredPktsThisAck: INET has no segment-boundary tracking once bytes enter the
        // (byte-range-based) rexmit-queue/send-queue model, so this approximates "packets"
        // the same way Linux's own tcp_skb_pcount (GSO/TSO segment counting, which INET
        // doesn't model either) ultimately reduces to for a non-offloaded sender: one MSS
        // of newly-delivered bytes per packet. Reuses the existing prrDeliveredMark
        // snapshot (process_RCV_SEGMENT, RFC 6937 PRR) rather than adding a second one.
        uint64_t deliveredBytesThisAck = state->deliveredBytes - state->prrDeliveredMark;
        uint32_t mss = state->snd_mss > 0 ? state->snd_mss : 1;
        uint32_t deliveredPktsThisAck = (uint32_t)((deliveredBytesThisAck + mss - 1) / mss);

        int delta = ((int)receivedAce - 5 - (int)(state->deliveredCePkts & 0x7)) & 0x7;
        int safeDelta = delta;
        if (deliveredPktsThisAck > 7) {
            // Naive delta can't distinguish "the counter wrapped around more than once"
            // from "it wrapped around once" when more than 8 packets were delivered in a
            // single ACK -- resolve against the actual delivered-packet count instead.
            safeDelta = (int)deliveredPktsThisAck - (((int)deliveredPktsThisAck - delta) & 0x7);
        }

        // Packets-acked EWMA (design reference: __tcp_accecn_process's pkts_acked_ewma,
        // tcp_input.c; PKTS_ACKED_WEIGHT=PKTS_ACKED_PREC=6 reimplemented here). Tracks
        // whether large ACKs are the NORM for this flow (receiver-side ACK
        // compression / GRO). When they are, a big single-ACK delivered-packet count
        // is expected and does NOT imply the mod-8 ACE counter wrapped, so the naive
        // delta -- not safeDelta -- is the correct CE count. Updated on every ACK.
        if (deliveredPktsThisAck > 0) {
            if (state->pktsAckedEwma == 0)
                state->pktsAckedEwma = deliveredPktsThisAck << 6; // PKTS_ACKED_PREC
            else {
                uint32_t e = state->pktsAckedEwma;
                e = (((e << 6) - e) + (deliveredPktsThisAck << 6)) >> 6; // weight 6
                state->pktsAckedEwma = std::min<uint32_t>(e, 0xFFFF);
            }
        }

        // AccECN TCP option: if this ACK also carried a valid AccECN
        // option (readHeaderOptions() already ran and set accEcnOptionCebDeltaValid,
        // before this function, for this same segment), its byte-exact CEB evidence can
        // corroborate naiveDelta vs. safeDelta -- resolveAceDelta() picks whichever
        // candidate's byte estimate is closer to the observed CE byte delta. Without the
        // option, the packet-count-only safeDelta is used as-is.
        int resolvedDelta = safeDelta;
        long cebDeltaForTrace = -1;
        if (state->accEcnOptionCebDeltaValid) {
            // Compute the delta AND advance the peerReportedCeBytes baseline together,
            // right here at the one place that actually consumes it -- readHeaderOptions()
            // deliberately left the baseline untouched (see its own comment and the state
            // field's) so this function's early-return path (an ACK beyond snd_max, above)
            // can never advance the baseline while discarding the delta it implies.
            uint32_t cebDelta = (state->accEcnOptionRawCeBytes - state->peerReportedCeBytes) & 0xFFFFFF;
            resolvedDelta = resolveAceDelta(delta, safeDelta, cebDelta);
            state->deliveredCeBytes += cebDelta;
            state->peerReportedCeBytes = state->accEcnOptionRawCeBytes;
            emit(deliveredCeBytesSignal, (unsigned long)state->deliveredCeBytes);
            cebDeltaForTrace = (long)cebDelta;
        }
        else if (deliveredPktsThisAck > 7 && state->pktsAckedEwma > (4u << 6)) {
            // No AccECN option to disambiguate, but this flow's ACKs routinely
            // cover many packets (EWMA above ACK_COMP_THRESH=4): the large
            // delivered-packet count is ACK compression, not an ACE counter wrap,
            // so the naive mod-8 delta is the correct CE count rather than safeDelta.
            resolvedDelta = delta;
        }

        state->deliveredCePkts += resolvedDelta;
        emit(deliveredCeSignal, (unsigned long)state->deliveredCePkts);
        EV_INFO << "AccECN ACE decode: receivedAce=" << (int)receivedAce
                << " deliveredPktsThisAck=" << deliveredPktsThisAck
                << " naiveDelta=" << delta << " safeDelta=" << safeDelta
                << " cebDeltaValid=" << state->accEcnOptionCebDeltaValid
                << " cebDelta=" << cebDeltaForTrace
                << " resolvedDelta=" << resolvedDelta
                << " deliveredCePkts=" << state->deliveredCePkts << "\n";
    }

    // ECT0/ECT1 delivered-byte accounting (tcpi_delivered_e0/e1_bytes). Unlike the
    // ACE-field CE-packet delta above, these cumulative byte counters advance on ANY
    // in-window ACK that carried a valid AccECN option, not only forward-progress
    // ACKs -- a pure/duplicate ACK simply repeats the same counter, so its delta is 0.
    // Gating this on forward progress (as the ACE block is) would drop the delta of a
    // final ACK that only advances the cumulative ACK past already-in-flight data.
    if (state->accEcnOptionE0DeltaValid) {
        state->deliveredE0Bytes += (state->accEcnOptionRawE0Bytes - state->peerReportedEct0Bytes) & 0xFFFFFF;
        state->peerReportedEct0Bytes = state->accEcnOptionRawE0Bytes;
    }
    if (state->accEcnOptionE1DeltaValid) {
        state->deliveredE1Bytes += (state->accEcnOptionRawE1Bytes - state->peerReportedEct1Bytes) & 0xFFFFFF;
        state->peerReportedEct1Bytes = state->accEcnOptionRawE1Bytes;
    }

    return true;
}

int TcpConnection::resolveAceDelta(int naiveDelta, int safeDelta, uint32_t cebByteDelta) const
{
    if (naiveDelta == safeDelta)
        return naiveDelta; // no ambiguity to resolve

    uint32_t mss = state->snd_mss > 0 ? state->snd_mss : 1;
    uint64_t naiveBytesEstimate = (uint64_t)naiveDelta * mss;
    uint64_t safeBytesEstimate = (uint64_t)safeDelta * mss;
    uint64_t naiveDiff = (cebByteDelta > naiveBytesEstimate) ? (cebByteDelta - naiveBytesEstimate) : (naiveBytesEstimate - cebByteDelta);
    uint64_t safeDiff = (cebByteDelta > safeBytesEstimate) ? (cebByteDelta - safeBytesEstimate) : (safeBytesEstimate - cebByteDelta);
    return (naiveDiff <= safeDiff) ? naiveDelta : safeDelta;
}

// ----

void TcpConnection::process_TIMEOUT_CONN_ESTAB()
{
    switch (fsm.getState()) {
        case TCP_S_SYN_RCVD:
        case TCP_S_SYN_SENT:
            // Nothing to do here. TIMEOUT_CONN_ESTAB event will automatically
            // take the connection to LISTEN or CLOSED, and cancel SYN-REXMIT timer.
            if (state->active) {
                // notify user if we're on the active side
                sendIndicationToApp(TCP_I_TIMED_OUT);
            }
            break;

        default:
            // We should not receive this timeout in this state.
            throw cRuntimeError(tcpMain, "Internal error: received CONN_ESTAB timeout in state %s",
                stateName(fsm.getState()));
    }
}

void TcpConnection::process_TIMEOUT_2MSL()
{
    //"
    // If the time-wait timeout expires on a connection delete the TCB,
    // enter the CLOSED state and return.
    //"
    switch (fsm.getState()) {
        case TCP_S_TIME_WAIT:
            // Nothing to do here. The TIMEOUT_2MSL event will automatically take
            // the connection to CLOSED. We already notified the user
            // (TCP_I_CLOSED) when we entered the TIME_WAIT state from CLOSING,
            // FIN_WAIT_1 or FIN_WAIT_2.
            break;

        default:
            // We should not receive this timeout in this state.
            throw cRuntimeError(tcpMain,
                "Internal error: received time-wait (2MSL) timeout in state %s",
                stateName(fsm.getState()));
    }
}

void TcpConnection::process_TIMEOUT_FIN_WAIT_2()
{
    switch (fsm.getState()) {
        case TCP_S_FIN_WAIT_2:
            // Nothing to do here. The TIMEOUT_FIN_WAIT_2 event will automatically take
            // the connection to CLOSED.
            break;

        default:
            // We should not receive this timeout in this state.
            throw cRuntimeError(tcpMain, "Internal error: received FIN_WAIT_2 timeout in state %s",
                stateName(fsm.getState()));
    }
}

void TcpConnection::startSynRexmitTimer()
{
    state->syn_rexmit_count = 0;
    // Linux retransmits the SYN/SYN-ACK on the same initial RTO as data (1s;
    // TCP_TIMEOUT_INIT), doubling per attempt. The initialRto parameter sets it.
    state->syn_rexmit_timeout = tcpMain->par("initialRto");
    rescheduleAfter(state->syn_rexmit_timeout, synRexmitTimer);
}

void TcpConnection::process_TIMEOUT_SYN_REXMIT(TcpEventCode& event)
{
    // Linux net.ipv4.tcp_syn_retries / TCP_SYNCNT: cap on SYN retransmissions
    // (read live so a runtime-injected sockopt takes effect); -1 keeps INET's
    // historical MAX_SYN_REXMIT_COUNT.
    int synRetries = tcpMain->par("synRetries");
    int maxSynRexmitCount = synRetries >= 0 ? synRetries : MAX_SYN_REXMIT_COUNT;
    if (++state->syn_rexmit_count > maxSynRexmitCount) {
        EV_INFO << "Retransmission count during connection setup exceeds " << maxSynRexmitCount << ", giving up\n";
        // Note ABORT will take the connection to closed, and cancel CONN-ESTAB timer as well
        event = TCP_E_ABORT;
        return;
    }

    // TCP Fast Open active blackhole detection: repeated SYN-REXMITs on a
    // connection whose SYN carried data suggest a middlebox is dropping/mangling
    // TFO SYN+data specifically (a plain-SYN retransmit would usually get through
    // sooner) -- matches the kernel's tcp_fastopen_active_should_disable() 3rd
    // (index-2) consecutive-timeout trigger.
    if (state->fastopenSynDataLen > 0 && state->syn_rexmit_count == TFO_BLACKHOLE_RTO_THRESHOLD)
        tcpMain->recordFastOpenBlackhole();

    EV_INFO << "Performing retransmission #" << state->syn_rexmit_count << "\n";

    // resend what's needed
    switch (fsm.getState()) {
        case TCP_S_SYN_SENT:
            sendSyn();
            break;

        case TCP_S_SYN_RCVD:
            sendSynAck();
            break;

        default:
            throw cRuntimeError(tcpMain, "Internal error: SYN-REXMIT timer expired while in state %s",
                stateName(fsm.getState()));
    }

    // reschedule timer
    state->syn_rexmit_timeout *= 2;

    if (state->syn_rexmit_timeout > TCP_TIMEOUT_SYN_REXMIT_MAX)
        state->syn_rexmit_timeout = TCP_TIMEOUT_SYN_REXMIT_MAX;

    scheduleAfter(state->syn_rexmit_timeout, synRexmitTimer);
}

//
// TODO
//"
// USER TIMEOUT
//
//    For any state if the user timeout expires, flush all queues, signal
//    the user "error:  connection aborted due to user timeout" in general
//    and for any outstanding calls, delete the TCB, enter the CLOSED
//    state and return.
//"

} // namespace tcp
} // namespace inet

