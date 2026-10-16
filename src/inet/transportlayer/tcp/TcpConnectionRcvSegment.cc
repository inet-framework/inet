//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2011 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <string.h>

#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"
#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpReceiveQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"
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

    // RFC 9293 section 3.10.7.1 (CLOSED STATE; SEG.LEN is from section 3.4):
    //"
    // all data in the incoming segment is discarded.  An incoming
    // segment containing a RST is discarded.  An incoming segment not
    // containing a RST causes a RST to be sent in response.  The
    // acknowledgment and sequence field values are selected to make the
    // reset sequence acceptable to the TCP endpoint that sent the offending
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

    emit(rcvSeqSignal, tcpHeader->getSequenceNo());
    emit(rcvAckSignal, tcpHeader->getAckNo());

    state->time_last_segment_received = simTime(); // the idle time of keepalive starts here

    emit(tcpRcvPayloadBytesSignal, int(tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>()));

    // snapshot delivered-bytes so consumers can read this segment's newly
    // acked+sacked bytes as deliveredBytes - prrDeliveredMark (RFC 6937 PRR input)
    state->prrDeliveredMark = state->deliveredBytes;
    state->accEcnTsProgress = false;

    // reset the per-segment D-SACK detection (RFC 2883 loss undo)
    state->dsackSeen = false;
    state->dsackBytes = 0;
    state->dsackEndSeq = 0;
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
        // RFC 9293 section 3.10.7.4 steps "First, check sequence number", "Second, check the RST bit", etc
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
    // a buffer that SO_RCVBUF locked cannot grow, so a socket already over its budget
    // takes nothing more until the application reads (Linux tcp_try_rmem_schedule());
    // the segment that first crosses the budget still gets in
    if (payloadLength > 0 && state->rcvbufLocked && state->rcvBufferSize > 0 && rcvBufOccupancy > state->rcvBufferSize)
        return false;
    if (seqLess(payloadSeq, firstSeq)) {
        long delta = firstSeq - payloadSeq;
        payloadSeq += delta;
        payloadLength -= delta;
    }
    // As Linux (tcp_data_queue()), with acceptBeyondBufferAtEmptyQueue: an in-order segment
    // at an empty receive queue is accepted whole, also beyond the buffer; the window is
    // then zero until the application reads
    if (state->acceptBeyondBufferAtEmptyQueue && payloadLength > 0 && payloadSeq == state->rcv_nxt
        && receiveQueue->getAmountOfBufferedBytes() == 0)
        return true;
    // a locked buffer counts its memory, not the bytes of its data (Linux
    // tcp_can_ingest()): within the buffer, a segment gets in whole, also when it ends
    // beyond the buffer
    if (payloadLength > 0 && state->rcvbufLocked && state->rcvBufferSize > 0)
        return seqLE(firstSeq, payloadSeq);
    uint32_t bufferSize = state->rcvBufferSize > 0 ? state->rcvBufferSize : state->maxRcvBuffer;
    return seqLE(firstSeq, payloadSeq) && seqLE(payloadSeq + payloadLength, firstSeq + bufferSize);
}

TcpEventCode TcpConnection::processSegment1stThru8th(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{

    // Delegates additional processing of ECN to the algorithm
    tcpAlgorithm->processEcnInEstablished();

    //
    // RFC 9293 section 3.10.7.4: First, check sequence number
    //

    bool acceptable = true;

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) { // Header options present? TCP_HEADER_OCTETS = 20
        // PAWS, RFC 7323 section 5.3: "R1) If there is a Timestamps option in the arriving
        // segment, SEG.TSval < TS.Recent, TS.Recent is valid (see later discussion), and if
        // the RST bit is not set, then treat the arriving segment as not acceptable".
        // TS.Recent is valid unless the connection was idle for more than 24 days (section
        // 5.5).
        const TcpOptionTimestamp *tsOption = state->ts_enabled && !tcpHeader->getRstBit() ? findTSOption(tcpHeader) : nullptr;
        if (tsOption != nullptr && seqLess(tsOption->getSenderTimestamp(), state->ts_recent)
            && (simTime() - state->ts_recent_stamp) <= PAWS_IDLE_TIME_THRESH)
        {
            EV_DETAIL << "PAWS: Segment is not acceptable, TSval=" << tsOption->getSenderTimestamp() << " < TS.Recent=" << state->ts_recent
                      << " in " << stateName(fsm.getState()) << " state received: dropping segment\n";
            acceptable = false;
        }

        readHeaderOptions(tcpHeader);
    }

    if (acceptable)
        acceptable = isSegmentAcceptable(tcpSegment, tcpHeader);

    if (acceptable)
        updateTsRecent(tcpHeader);

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
                // A retransmitted SYN in SYN_RCVD shows that our SYN-ACK was lost:
                // Linux re-sends the SYN-ACK (tcp_check_req()), not a plain ACK. Only
                // for a passive open, which is a request socket in Linux: in a
                // simultaneous open, Linux answers with the plain ACK below, and so
                // it does for a SYN-ACK, which completes a simultaneous open.
                // AccECN client (RFC 9768 section 3.2.2.1): "The TCP Client MUST also use
                // the handshake encoding for the pure ACK of any retransmitted SYN/ACK"
                if (state->accEcnNegotiated && tcpHeader->getAckBit()) {
                    state->accEcnReflectCodepoint = receivedEcnCodepoint(tcpSegment);
                    state->accEcnReflectAce = true;
                }
                if (fsm.getState() == TCP_S_SYN_RCVD && !state->active && !tcpHeader->getAckBit()) {
                    EV_DETAIL << "Re-sending SYN-ACK for the retransmitted SYN\n";
                    // AccECN server (RFC 9768 section 3.1.5): the SYN-ACK feeds back the IP
                    // ECN codepoint of the latest SYN, and after a SYN with (AE,CWR,ECE) =
                    // (0,0,0), the server "MUST NOT set ECT on any packet for the rest of the
                    // connection"
                    if (state->accEcnNegotiated) {
                        state->accEcnReflectCodepoint = receivedEcnCodepoint(tcpSegment);
                        if (!tcpHeader->getAeBit() && !tcpHeader->getCwrBit() && !tcpHeader->getEceBit())
                            state->accEcnNoEct = true;
                    }
                    // it counts as a SYN-ACK retransmission (Linux increments
                    // num_retrans in inet_rtx_syn_ack())
                    state->syn_rexmit_count++;
                    sendSynAck();
                    state->rcv_naseg++;
                    emit(rcvNASegSignal, state->rcv_naseg);
                    return TCP_E_IGNORE;
                }
            }
            else if (payloadLength + tcpHeader->getSynFinLen() > 0 && state->sack_enabled
                     && seqLess(tcpHeader->getSequenceNo(), state->rcv_nxt)) {
                // AccECN (RFC 9768 section 3.2.3.2.2): a host "MAY detect loss of ACKs
                // carrying the AccECN Option by detecting whether the acknowledged data
                // always reappears as a retransmission. In such cases, the host SHOULD
                // disable the sending of the AccECN Option for this half-connection." As
                // Linux (tcp_rcv_spurious_retrans()): the data of the D-SACK block that went
                // out with the option arrives again, so that ACK was lost too.
                if (state->accEcnOptSentWithDsack && tcpHeader->getSequenceNo() == state->accEcnSentDsackStart
                    && !state->accEcnOptFailSend)
                {
                    state->accEcnOptFailSend = true;
                    EV_INFO << "AccECN: the data of the D-SACK block that went with the AccECN option arrived again: no more AccECN options\n";
                }
                // Linux tcp_send_dupack: ANY old data (seq before rcv_nxt) earns a
                // D-SACK, including a duplicate ending exactly at rcv_nxt -- the
                // range's right edge is capped at rcv_nxt by addSacks. SEG.LEN
                // counts SYN and FIN (Linux end_seq).
                state->start_seqno = tcpHeader->getSequenceNo();
                state->end_seqno = tcpHeader->getSequenceNo() + payloadLength + tcpHeader->getSynFinLen();
                state->snd_dsack = true;
                EV_DETAIL << "SND_D-SACK SET (dupseg rcvd)\n";
            }

            // AccECN, accEcnOptionPolicy="linux": Linux tests the acceptability against
            // the left edge of the window that it advertised last (tcp_sequence()), so
            // a duplicate that ends at RCV.NXT passes, and the counters count it
            if (state->accEcnNegotiated && tcpMain->accEcnOptionLinux && payloadLength > 0
                && tcpHeader->getSequenceNo() + payloadLength == state->rcv_nxt)
                countAccEcnReceived(tcpSegment, payloadLength);

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

    // AccECN (RFC 9768 section 3.2): section 4: "only 'Acceptable' packets contribute
    // to the ECN counters"
    if (state->accEcnNegotiated)
        countAccEcnReceived(tcpSegment, payloadLength);

    // ECN (an AccECN connection uses the CWR bit for the ACE field)
    if (tcpHeader->getCwrBit() == true && !state->accEcnNegotiated) {
        EV_INFO << "Received CWR... Leaving ecnEcho State\n";
        state->ecnEchoState = false;
    }

    //
    // RFC 9293 section 3.10.7.4: Second, check the RST bit:
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

    // RFC 9293 section 3.10.7.4: Third, check security
    // This step is ignored.

    //
    // RFC 9293 section 3.10.7.4: Fourth, check the SYN bit:
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
    // RFC 9293 section 3.10.7.4: Fifth, check the ACK field:
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

        // the algorithms count with the effective MSS; until segments are sized
        // against the option space, it is the negotiated MSS
        state->snd_effmss = calculateEffectiveMss();

        // Seed the RTT estimator from the handshake RTT (Linux measures the
        // SYN<->SYN-ACK exchange via tcp_ack_update_rtt/tcp_synack_rtt_meas and
        // enters ESTABLISHED with srtt/rttvar -- and hence the first RTO -- already
        // RTT-scaled instead of the initial default). Karn: skipped if our handshake
        // segment was retransmitted.
        if (state->seedRttFromHandshake && state->syn_rexmit_count == 0 && state->handshakeSentTime >= SIMTIME_ZERO)
            tcpAlgorithm->rttMeasurementComplete(state->handshakeSentTime, simTime());

        // notify tcpAlgorithm and app layer; a TCP Fast Open connection did so at its SYN
        if (!state->fastopenAccelerated) {
            initMtuProbing(); // before the initial window (Linux tcp_init_transfer())
            tcpAlgorithm->established(false);

            if (isToBeAccepted())
                sendAvailableIndicationToApp();
            else
                sendEstabIndicationToApp();
        }

        // The text of a SYN without Fast Open waits in the receive queue since LISTEN
        // (RFC 9293: "queued for processing later"); it is processed now, as the data of
        // a segment: RCV.NXT moves over it, it goes to the application, and an ACK follows.
        if (seqGreater(receiveQueue->getRcvNxt(), state->rcv_nxt)) {
            EV_INFO << "Processing the text of the SYN: " << receiveQueue->getRcvNxt() - state->rcv_nxt << " bytes\n";
            state->rcv_nxt = receiveQueue->getRcvNxt();
            if (!isToBeAccepted())
                sendAvailableDataToApp();
            tcpAlgorithm->receiveSeqChanged();
        }

        // A SYN-ACK that completes a simultaneous open repeats the SYN that this
        // side already received: report its one sequence number by a D-SACK, with
        // an immediate ACK, as Linux tcp_data_queue() does for a segment below
        // rcv_nxt (RFC 2883)
        if (tcpHeader->getSynBit() && state->sack_enabled && state->dsack_enabled
                && seqLess(tcpHeader->getSequenceNo(), state->rcv_nxt))
        {
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
    // TCP must be able to process acceptable acknowledgments, however please note RFC 9293 section 3.10.7.4 (fifth step, LAST-ACK STATE):
    // "The only thing that can arrive in this state is an
    //  acknowledgment of our FIN.  If our FIN is now
    //  acknowledged, delete the TCB, enter the CLOSED state, and
    //  return."
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
    // RFC 9293 section 3.10.7.4: Sixth, check the URG bit:
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

        // urgent data is not supported, except the out-of-band byte of Linux
        if (state->urgentOutOfBand)
            processUrgentPointer(tcpHeader->getSequenceNo() + tcpHeader->getUrgentPointer());
    }

    //
    // RFC 9293: Seventh, process the segment text
    //
    uint32_t old_rcv_nxt = state->rcv_nxt; // if rcv_nxt changes, we need to send/schedule an ACK

    // RFC 1122 section 4.2.2.13: after a "half-duplex" close (abortOnDataAfterClose,
    // the full close of Linux), the application will not read again, so new data in
    // FIN_WAIT_1/2 would be dropped while the peer thinks that it was delivered: "its
    // TCP SHOULD send a RST to show that data was lost" (Linux TCPABORTONDATA). The
    // RST takes its sequence number from the ACK of the segment, as Linux
    // tcp_v4_send_reset() does. The application asked for the close, so it gets
    // the CLOSED indication that its socket waits for.
    if ((fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2)
            && state->rcvShutdown && payloadLength > 0
            && seqGreater(tcpHeader->getSequenceNo() + payloadLength, state->rcv_nxt))
    {
        EV_INFO << "New data after a full close: resetting the connection (RFC 1122 4.2.2.13)\n";
        sendRst(tcpHeader->getAckNo());
        sendIndicationToApp(TCP_I_CLOSED);
        return TCP_E_ABORT;
    }

    // D-SACK bookkeeping (RFC 2883): first duplicated range of this segment,
    // captured just before the insert merges the regions.
    bool dupRangeFound = false;
    uint32_t dupStart = 0, dupEnd = 0;

    if (fsm.getState() == TCP_S_SYN_RCVD || fsm.getState() == TCP_S_ESTABLISHED ||
        fsm.getState() == TCP_S_FIN_WAIT_1 || fsm.getState() == TCP_S_FIN_WAIT_2)
    {
        //"
        // Once in the ESTABLISHED state, it is possible to deliver
        // segment data to user RECEIVE buffers.  Data from segments
        // can be moved into buffers until either the buffer is full or
        // the segment is empty.  If the segment empties and carries a
        // PUSH flag, then the user is informed, when the buffer is
        // returned, that a PUSH has been received.
        //
        // When the TCP endpoint takes responsibility for delivering
        // the data to the user, it must also acknowledge the receipt
        // of the data.
        //
        // Once the TCP endpoint takes responsibility for the data, it
        // advances RCV.NXT over the data accepted, and adjusts RCV.WND
        // as appropriate to the current buffer availability.  The
        // total of RCV.NXT and RCV.WND should not be reduced.
        //
        // A TCP implementation MAY send an ACK segment acknowledging
        // RCV.NXT when a valid segment arrives that is in the window
        // but not at the left window edge (MAY-13).
        //
        // Please note the window management suggestions in
        // Section 3.8.
        //
        // Send an acknowledgment of the form:
        //
        //   <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
        //
        // This acknowledgment should be piggybacked on a segment being
        // transmitted if possible without incurring undue delay.
        //"

        // the receive RTT of Linux, after the ACK and before the data (tcp_rcv_established())
        if (fsm.getState() == TCP_S_ESTABLISHED)
            measureRcvRttTs(tcpHeader, payloadLength);

        // only data beyond RCV.NXT: a segment of old data comes here only for its FIN,
        // which the eighth step processes
        if (payloadLength > 0 && seqGreater(tcpHeader->getSequenceNo() + payloadLength, state->rcv_nxt)) {
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
                // D-SACK (RFC 2883): the part of the segment below rcv_nxt is a
                // duplicate (Linux tcp_data_queue: D-SACK [seq, rcv_nxt)); otherwise
                // find the first already-buffered range this segment duplicates
                // BEFORE the insert merges the regions (Linux reports it via
                // tcp_dsack_set/tcp_dsack_extend as tcp_ofo_queue drains the
                // out-of-order queue over a gap-filling segment).
                if (state->sack_enabled && state->dsack_enabled && payloadLength > 0) {
                    if (seqLess(tcpHeader->getSequenceNo(), state->rcv_nxt)) {
                        dupStart = tcpHeader->getSequenceNo();
                        dupEnd = state->rcv_nxt;
                        dupRangeFound = true;
                    }
                    else
                        dupRangeFound = receiveQueue->findFirstDuplicateRange(tcpHeader->getSequenceNo(),
                                tcpHeader->getSequenceNo() + payloadLength, dupStart, dupEnd);
                }
                uint32_t oldRcvNxt = state->rcv_nxt;
                state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, tcpHeader);
                // in-order data (Linux tcp_event_data_recv() in tcp_data_queue())
                if (!seqGreater(tcpHeader->getSequenceNo(), oldRcvNxt) && seqGreater(state->rcv_nxt, oldRcvNxt))
                    measureRcvRtt();
                if (payloadLength > 0) {
                    // rcv_mwnd_seq is still the highest edge before this arrival: a
                    // segment beyond it came in by acceptBeyondBufferAtEmptyQueue
                    if (seqGreater(tcpHeader->getSequenceNo() + payloadLength, state->rcv_mwnd_seq))
                        overWindowAcceptPending = true;
                    chargeRcvBuffer(payloadLength);
                    // windowAutoTuning, as Linux (tcp_clamp_window()): when the charged
                    // segments outgrow the receive buffer of a connection that the
                    // application owns, the buffer grows; a connection that the
                    // application did not accept yet keeps its buffer, and so does a
                    // buffer that SO_RCVBUF locked
                    if (state->rcv_ssthresh > 0 && !state->rcvbufLocked && appOwned && !isToBeAccepted() && rcvBufOccupancy > state->rcvBufferSize
                        && state->rcvBufferSize < state->rcvBufferMax)
                    {
                        state->rcvBufferSize = (uint32_t)std::min<uint64_t>(rcvBufOccupancy, state->rcvBufferMax);
                        if (state->maxRcvBuffer < state->rcvBufferSize)
                            state->maxRcvBuffer = state->rcvBufferSize;
                        EV_DETAIL << "Window auto-tuning: the receive buffer grows to " << state->rcvBufferSize << "\n";
                    }
                    if (seqGreater(tcpHeader->getSequenceNo(), oldRcvNxt))
                        growRcvBufferForOutOfOrderData(tcpHeader->getSequenceNo() + payloadLength);
                }

                // windowAutoTuning, as Linux (tcp_grow_window(), also for out-of-order
                // data): each data segment grows the offer by max(2 * MSS, 2 * its
                // length), up to the clamp
                if (state->rcv_ssthresh > 0 && state->rcv_ssthresh < state->window_clamp && payloadLength > 0) {
                    uint32_t increase = std::max<uint32_t>(2 * state->snd_mss, 2 * payloadLength);
                    state->rcv_ssthresh += std::min(state->window_clamp - state->rcv_ssthresh, increase);
                    EV_DETAIL << "Window auto-tuning: the offer grows to " << state->rcv_ssthresh << "\n";
                }

                // RFC 5681, page 8:
                // "3.2 Fast Retransmit/Fast Recovery
                // (...)
                // In addition, a TCP receiver SHOULD send an immediate ACK
                // when the incoming segment fills in all or part of a gap in the
                // sequence space."
                // The segment filled all of a gap if rcv_nxt moved past its end, and
                // the first part of a gap if out-of-order data stays above rcv_nxt.
                // Set ack_now before the notification below: a data segment that it
                // sends carries the ACK and clears ack_now.
                if (state->rcv_nxt != old_rcv_nxt && (tcpHeader->getSequenceNo() + payloadLength != state->rcv_nxt || receiveQueue->hasOutOfOrderData()))
                    state->ack_now = true;

                if (seqGreater(state->snd_una, old_snd_una)) {
                    // notify
                    tcpAlgorithm->receivedAckForUnackedData(old_snd_una);

                    // in the receivedAckForUnackedData we need the old value
                    state->dupacks = 0;
                    state->limitedTransmitBytes = 0;

                    emit(dupAcksSignal, state->dupacks);
                }

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
                // a locked buffer answers at once with a zero window, as Linux
                // (ICSK_ACK_NOMEM, ICSK_ACK_NOW), so the peer waits until the application reads
                if (state->rcvbufLocked) {
                    state->ackNomem = true;
                    sendAck();
                }
                return TCP_E_IGNORE;
            }
        }
    }

    //
    // RFC 9293 section 3.10.7.4: Eighth, check the FIN bit:
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

        // Note: RFC 9293 section 3.10.7.4 assumes that the segment begins at
        // RCV.NXT, and says to hold segments with higher sequence numbers for
        // later processing. Here, if the segment is "above sequence"
        // (ie. RCV.NXT < SEG.SEQ), we cannot advance RCV.NXT over the FIN.
        // Instead we remember this sequence number and do it later.
        uint32_t fin_seq = (uint32_t)tcpHeader->getSequenceNo() + (uint32_t)payloadLength;

        if (state->rcv_nxt == fin_seq) {
            // advance rcv_nxt over FIN now; remember where the FIN is, because the
            // data that the application can still read ends there
            EV_INFO << "FIN arrived, advancing rcv_nxt over the FIN\n";
            state->fin_rcvd = true;
            state->rcv_fin_seq = fin_seq;
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
                // data that was already received -- report the first duplicated
                // range as a D-SACK block; addSacks() appends the still-missing
                // out-of-order blocks (if any) after it (Linux tcp_ofo_queue ->
                // tcp_dsack_extend).
                state->start_seqno = dupStart;
                state->end_seqno = dupEnd;
                state->snd_dsack = true;
                EV_DETAIL << "SND_D-SACK SET (segment duplicates received range [" << dupStart << ".." << dupEnd << "))\n";
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

        // the arrival of the last in-order data, also data that fills a hole, but not a
        // FIN alone (Linux icsk_ack.lrcvtime, which tcp_event_data_recv() sets)
        if (payloadLength > 0)
            state->lastDataRecvTime = simTime();
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
    state->rcv_mwnd_seq = state->rcv_adv;

    emit(rcvAdvSignal, state->rcv_adv);

    state->irs = tcpHeader->getSequenceNo();
    receiveQueue->init(state->rcv_nxt); // FIXME may init twice...
    selectInitialSeqNum();
    resolveAddressFamilyMss(); // before the MSS option of the peer limits it

    // although not mentioned in RFC 9293 section 3.10.7.2, seems like we have to pick up
    // initial snd_wnd from the segment here.
    updateWndInfo(tcpHeader, true);

    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) // Header options present?
        readHeaderOptions(tcpHeader);
    applyDefaultSendMss(tcpHeader, srcAddr);

    // Linux tcp_syncookies=2 (syncookiesAlways): the connection is built again from
    // the cookie at the handshake ACK, and the 2-bit MSS field of the cookie
    // quantizes the MSS of the peer down to the IPv4 msstab of
    // net/ipv4/syncookies.c: the largest entry not above it. Window scale, SACK and
    // timestamps come through the timestamp encoding and stay exact.
    if (tcpMain->par("syncookiesAlways").boolValue() && state->snd_mss > 536) {
        static const uint32_t msstab[] = { 536, 1300, 1440, 1460 };
        uint32_t clamped = msstab[0];
        for (uint32_t entry : msstab)
            if (entry <= state->snd_mss)
                clamped = entry;
        if (clamped < state->snd_mss) {
            EV_DETAIL << "syncookiesAlways: peer MSS " << state->snd_mss << " quantized to " << clamped << "\n";
            state->snd_mss = clamped;
            state->snd_effmss = calculateEffectiveMss();
        }
    }

    state->ack_now = true;

    // ECN. A server with AccECN (RFC 9768 section 3.1.1) negotiates AccECN for a SYN
    // with (AE,CWR,ECE) = (1,1,1), and also for each combination other than (0,0,0)
    // and the classic (0,1,1) (section 3.1.3: "the Server MUST negotiate the use of
    // AccECN as if the three flags had been set to (1,1,1)"; Linux
    // tcp_accecn_syn_requested()). The SYN-ACK feeds back the IP ECN codepoint of the
    // SYN. A server without AccECN reads a SYN with ECE and CWR, also the (1,1,1) of
    // AccECN, as an ECN-setup SYN (Table 2: the SYN-ACK of an ECN server is (0,0,1)).
    uint8_t synAce = (tcpHeader->getAeBit() ? 4 : 0) | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0);
    bool accEcnServer = state->ecnMode == TCP_ECN_MODE_ACCECN || state->ecnMode == TCP_ECN_MODE_ACCECN_PASSIVE;
    if (accEcnServer && synAce != 0 && synAce != 3) {
        state->endPointIsWillingECN = true;
        state->accEcnNegotiated = true;
        state->accEcnReflectCodepoint = receivedEcnCodepoint(tcpSegment);
        EV << "AccECN-setup SYN packet received (ACE=" << (int)synAce << ", IP ECN " << state->accEcnReflectCodepoint << ")\n";
    }
    else if (tcpHeader->getEceBit() == true && tcpHeader->getCwrBit() == true) {
        state->endPointIsWillingECN = true;
        EV << "ECN-setup SYN packet received\n";
    }

    // TCP Fast Open (RFC 7413 section 4.2.2), server: readHeaderOptions() checked the
    // cookie. A valid cookie makes this a Fast Open connection: the data of the SYN
    // goes into the receive queue after the SYN, so that the SYN-ACK acknowledges the
    // SYN and the data, and the application gets the connection and the data now
    // ("Buffer the data and notify the application"). With a Fast Open option but
    // no valid cookie, the SYN-ACK acknowledges only the SYN. As Linux can
    // (fastopenAcceptWithoutCookie), a SYN with data or a Fast Open option is
    // accepted without a valid cookie, and the SYN-ACK gives no cookie.
    bool fastOpenOption = state->fastopenCookieRequested || state->fastopenCookieValid || state->fastopenSendCookieOption;
    bool synData = B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength();
    // RFC 7413 section 4.2: over the limit of pending Fast Open connections, "the
    // server MUST disable TFO for all new connection requests": no cookie, and the
    // data of the SYN is not acknowledged (section 5.1)
    if (state->fastopenServerEnabled && (fastOpenOption || synData)
        && tcpMain->getNumPendingFastOpenRequests(localAddr, localPort) >= (int)tcpMain->par("fastopenMaxPendingRequests"))
    {
        EV_INFO << "Fast Open: the limit of pending Fast Open connections is reached, no Fast Open for this SYN\n";
        state->fastopenCookieValid = false;
        state->fastopenSendCookieOption = false;
        state->fastopenAcceptWithoutCookie = false;
    }
    bool acceptWithoutCookie = state->fastopenAcceptWithoutCookie && (synData || fastOpenOption);
    if (state->fastopenServerEnabled && (state->fastopenCookieValid || acceptWithoutCookie)) {
        if (!state->fastopenCookieValid)
            state->fastopenSendCookieOption = false;
        state->fastopenAccelerated = true;
        B payloadLength = B(tcpSegment->getByteLength()) - tcpHeader->getHeaderLength();
        updateRcvQueueVars();
        if (payloadLength > B(0) && hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) {
            // the SYN takes one sequence number, so the data starts at SEG.SEQ+1
            auto dataHeader = staticPtrCast<TcpHeader>(tcpHeader->dupShared());
            dataHeader->setSequenceNo(tcpHeader->getSequenceNo() + 1);
            state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, dataHeader);
            updateRcvQueueVars();
            state->fastopenSynDataAccepted = true;
            EV_INFO << "Fast Open: accepting " << payloadLength << " of SYN data before the handshake completes\n";
        }
        // a forked connection gets the data with the ACCEPT of the application
        if (isToBeAccepted())
            sendAvailableIndicationToApp();
        else {
            sendEstabIndicationToApp();
            sendAvailableDataToApp();
        }
    }

    sendSynAck();

    if (state->fastopenAccelerated) {
        // RFC 7413 section 4.2.2, step 6: "If the FastOpened flag is set, the server
        // MUST follow [RFC5681] (based on [RFC3390]) to set the initial congestion
        // window for sending more data packets". The algorithm starts now, not at
        // the ACK of the SYN-ACK (as Linux starts the transfer of the child socket,
        // tcp_init_transfer()), so that the application can send in SYN_RCVD. It
        // starts after the SYN-ACK, whose options settle SACK and the timestamps:
        // the recovery that the algorithm chooses depends on SACK.
        state->snd_effmss = calculateEffectiveMss();
        initMtuProbing(); // before the initial window (Linux tcp_init_transfer())
        tcpAlgorithm->established(false);
    }

    startSynRexmitTimer();

    //"
    // Note that any other incoming control or data (combined with SYN)
    // will be processed in the SYN-RECEIVED state, but processing of SYN
    // and ACK should not be repeated.
    //"
    // We don't send text in SYN or SYN+ACK, but accept it. Otherwise
    // there isn't much left to do: RST, SYN, ACK, FIN got processed already,
    // so there's only URG and PSH left to handle.
    //
    // The text of a SYN with a Fast Open option was accepted above, or it is
    // not acknowledged.
    //
    // dropSynData, as Linux tcp_conn_request(): the data of such a SYN is dropped, and
    // the client sends it again after the SYN-ACK, which acknowledges only the SYN
    if (!fastOpenOption && !state->fastopenAccelerated && synData && state->dropSynData)
        EV_INFO << "Dropping the text of the SYN (dropSynData)\n";
    else if (!fastOpenOption && !state->fastopenAccelerated && synData) {
        updateRcvQueueVars();

        // RFC 9293: "any other control or text should be queued for processing later".
        // The SYN takes SEG.SEQ, so the text starts at SEG.SEQ+1; RCV.NXT moves over it
        // when the connection reaches ESTABLISHED.
        auto textHeader = staticPtrCast<TcpHeader>(tcpHeader->dupShared());
        textHeader->setSequenceNo(tcpHeader->getSequenceNo() + 1);
        if (hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, textHeader)) { // enough freeRcvBuffer in rcvQueue for new segment?
            receiveQueue->insertBytesFromSegment(tcpSegment, textHeader);
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
            if (tcpHeader->getRstBit())
                EV_DETAIL << "ACK+RST bit set but wrong AckNo, ignored\n";
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
        // RFC 7323, Linux tcp_rcv_synsent_state_process() (PAWSACTIVEREJECTED): a
        // SYN-ACK whose TSecr does not echo a timestamp that this connection sent,
        // between the first SYN and now on its timestamp clock, gets an RST
        // <SEQ=SEG.ACK><CTL=RST>, and the connection stays in SYN_SENT. The lower
        // bound is the first SYN (Linux tp->retrans_stamp): a SYN-ACK to an earlier
        // copy of the SYN is valid. The check comes before the SYN-ACK changes any
        // state, as in Linux.
        if (tcpHeader->getAckBit() && state->ts_support && state->firstSynSentTime >= SIMTIME_ZERO) {
            for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
                auto ts = dynamic_cast<const TcpOptionTimestamp *>(tcpHeader->getHeaderOption(i));
                if (ts == nullptr || ts->getEchoedTimestamp() == 0)
                    continue;
                uint32_t tsecr = ts->getEchoedTimestamp();
                uint32_t tsLow = convertSimtimeToTS(state->firstSynSentTime);
                uint32_t tsHigh = convertSimtimeToTS(simTime());
                if (seqLess(tsecr, tsLow) || seqGreater(tsecr, tsHigh)) {
                    EV_WARN << "SYN-ACK TSecr " << tsecr << " outside [" << tsLow << ", " << tsHigh
                            << "]: sending RST (PAWSACTIVEREJECTED)\n";
                    sendRst(tcpHeader->getAckNo());
                    return TCP_E_IGNORE;
                }
            }
        }

        //
        //   RCV.NXT is set to SEG.SEQ+1, IRS is set to
        //   SEG.SEQ.  SND.UNA should be advanced to equal SEG.ACK (if there
        //   is an ACK), and any segments on the retransmission queue which
        //   are thereby acknowledged should be removed.
        //
        state->rcv_nxt = tcpHeader->getSequenceNo() + 1;
        state->rcv_adv = state->rcv_nxt + state->rcv_wnd;
        state->rcv_mwnd_seq = state->rcv_adv;

        emit(rcvAdvSignal, state->rcv_adv);

        state->irs = tcpHeader->getSequenceNo();
        receiveQueue->init(state->rcv_nxt);

        if (tcpHeader->getAckBit()) {
            state->snd_una = tcpHeader->getAckNo();
            sendQueue->discardUpTo(state->snd_una);

            rexmitQueue->discardUpTo(state->snd_una);

            // although not mentioned in RFC 9293 section 3.10.7.3, seems like we have to pick up
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

            // TCP Fast Open, client: the SYN-ACK acknowledged all the data of the SYN,
            // so the path has no blackhole
            if (state->fastopenSynDataLen > 0 && seqGE(state->snd_una, state->iss + 1 + state->fastopenSynDataLen)) {
                state->fastopenSynDataAccepted = true;
                tcpMain->resetFastOpenBlackhole();
            }

            // RFC 9293, section 3.10.7.3: "If there are other controls or text in
            // the segment, then continue processing at the sixth step under Section
            // 3.10.7.4 where the URG bit is checked". Those steps deal with URG, the
            // segment text (and PSH), and the FIN. URG and PSH are not supported.
            // The text goes into the receive queue and rcv_nxt moves over it, and
            // the FIN takes the connection on to CLOSE-WAIT, so that the ACK of the
            // SYN also acknowledges them.
            bool synAckFin = tcpHeader->getFinBit();

            if (B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength()) {
                updateRcvQueueVars();

                if (hasEnoughSpaceForSegmentInReceiveQueue(tcpSegment, tcpHeader)) { // enough freeRcvBuffer in rcvQueue for new segment?
                    // the SYN takes one sequence number, so the text starts at
                    // SEG.SEQ+1: the receive queue gets a copy of the header with
                    // that sequence number
                    auto textHeader = staticPtrCast<TcpHeader>(tcpHeader->dupShared());
                    textHeader->setSequenceNo(tcpHeader->getSequenceNo() + 1);
                    state->rcv_nxt = receiveQueue->insertBytesFromSegment(tcpSegment, textHeader);
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
            applyDefaultSendMss(tcpHeader, remoteAddr);

            // TCP Fast Open, client: a cookie request of a first SYN that the SYN-ACK did
            // not answer suggests a server that knows only the experimental option, so
            // the next request uses the other option, once (Linux
            // tcp_rcv_fastopen_synack()). After a retransmitted SYN, which carries no
            // option, the SYN-ACK tells nothing.
            if (state->fastopenExpOptionEnabled && state->fastopenCookieRequestPending && state->fastopenSynCarriedOption
                && state->syn_rexmit_count == 0)
            {
                bool answered = false;
                for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
                    auto option = tcpHeader->getHeaderOption(i);
                    if (option->getKind() == TCPOPTION_TCP_FASTOPEN || dynamic_cast<const TcpOptionTcpFastOpenExp *>(option))
                        answered = true;
                }
                if (!answered) {
                    bool usedExpOption = tcpMain->getFastOpenUseExpOption(remoteAddr);
                    tcpMain->noteFastOpenCookieRequestUnanswered(remoteAddr, usedExpOption);
                    EV_INFO << "Fast Open: the cookie request (kind " << (usedExpOption ? 254 : 34) << ") had no answer, the next one uses kind "
                            << (tcpMain->getFastOpenUseExpOption(remoteAddr) ? 254 : 34) << "\n";
                }
            }

            // TCP Fast Open (RFC 7413 section 4.2.2), client step 1: the MSS option of
            // the SYN-ACK updates the cache, also without a cookie
            // (readHeaderOptions() cached the cookie)
            if (state->fastopenRequested && state->peerAdvertisedMss > 0)
                tcpMain->updateFastOpenCachedMss(remoteAddr, state->peerAdvertisedMss);

            // TCP Fast Open (RFC 7413 section 4.2.2): "The client SHOULD retransmit
            // any unacknowledged data in the first ACK packet". snd_nxt goes back to
            // the first byte that the SYN-ACK did not acknowledge, afterRto keeps it
            // there for sendData(), and the bytes are no longer in flight.
            if (state->fastopenSynDataLen > 0 && seqLess(state->snd_una, state->snd_max)) {
                EV_INFO << "Fast Open: the SYN-ACK did not acknowledge the SYN data [" << state->snd_una << ", "
                        << state->snd_max << "), sending it with the ACK\n";
                state->snd_nxt = state->snd_una;
                state->afterRto = true;
                rexmitQueue->markLost(state->snd_una, state->snd_max);
            }

            // AccECN (RFC 9768 Table 2): the flags of the SYN-ACK decide the mode, before
            // established() sends the ACK of the SYN-ACK. (0,0,0) and (1,1,1): no ECN;
            // (0,0,1): classic ECN, which the ECN part after it completes; each other value:
            // AccECN, and it feeds back the IP ECN codepoint of our SYN. The ACK of the
            // SYN-ACK feeds back the codepoint of the SYN-ACK (Table 3), and a CE-marked
            // SYN-ACK counts as the first CE mark (section 3.2.2.2: r.cep becomes 6).
            if (state->aeSynSent) {
                uint8_t synAckAce = (tcpHeader->getAeBit() ? 4 : 0) | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0);
                if (synAckAce == 1)
                    state->ecnSynSent = true;
                else if (synAckAce != 0 && synAckAce != 7) {
                    state->accEcnNegotiated = true;
                    state->ect = true;
                    state->accEcnReflectCodepoint = receivedEcnCodepoint(tcpSegment);
                    state->accEcnReflectAce = true;
                    if (state->accEcnReflectCodepoint == IP_ECN_CE)
                        state->rcvCePkts = 1;
                    // (1,1,0) reports a CE-marked SYN: s.cep stays 5, but "the TCP
                    // Client still triggers the congestion control actions necessary
                    // to respond to the CE feedback" (section 3.2.2.2); the algorithm
                    // does it at the first ACK of data
                    if (synAckAce == 6)
                        state->gotEce = true;
                    EV << "AccECN-setup SYN-ACK packet received (ACE=" << (int)synAckAce << ", IP ECN "
                       << state->accEcnReflectCodepoint << ")... AccECN is enabled.\n";
                }
                state->aeSynSent = false;
            }

            // ECN, also before established(), which can send data at once: a segment
            // after the SYN-ACK carries the ECN state of the connection
            if (state->ecnSynSent) {
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
            else if (!state->accEcnNegotiated) {
                state->ect = false;
                if (tcpHeader->getEceBit() && !tcpHeader->getCwrBit())
                    EV << "ECN-setup SYN-ACK packet was received... ECN is disabled.\n";
            }

            // notify tcpAlgorithm (it has to send ACK of SYN) and app layer
            state->ack_now = true;
            state->snd_effmss = calculateEffectiveMss();
            // Seed the RTT estimator from the handshake RTT (Linux measures the
            // SYN<->SYN-ACK exchange via tcp_ack_update_rtt/tcp_synack_rtt_meas and
            // enters ESTABLISHED with srtt/rttvar -- and hence the first RTO -- already
            // RTT-scaled instead of the initial default). Karn: skipped if our handshake
            // segment was retransmitted.
            if (state->seedRttFromHandshake && state->syn_rexmit_count == 0 && state->handshakeSentTime >= SIMTIME_ZERO)
                tcpAlgorithm->rttMeasurementComplete(state->handshakeSentTime, simTime());

            if (synAckFin) {
                EV_INFO << "FIN on the SYN-ACK: advancing rcv_nxt over the FIN\n";
                state->fin_rcvd = true;
                state->rcv_fin_seq = state->rcv_nxt;
                state->rcv_nxt = state->rcv_fin_seq + 1;
            }

            initMtuProbing(); // before the initial window (Linux tcp_init_transfer())
            tcpAlgorithm->established(true);
            tcpMain->emit(Tcp::tcpConnectionAddedSignal, this);
            sendEstabIndicationToApp();
            // the text of the SYN-ACK: the data transfer states deliver the text of
            // a segment, but this segment reaches ESTABLISHED only on return
            sendAvailableDataToApp();

            if (synAckFin) {
                // ESTABLISHED now, and the FIN takes the connection on to CLOSE-WAIT
                // by the transition of the caller
                performStateTransition(TCP_E_RCV_SYN_ACK);
                return TCP_E_RCV_FIN;
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
        // simultaneous open: read the options of the crossing SYN before the
        // SYN-ACK is built, as processSynInListen() does; else the MSS of the
        // peer stays unknown, and SACK, window scale and timestamps stay off
        if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH)
            readHeaderOptions(tcpHeader);
        applyDefaultSendMss(tcpHeader, remoteAddr);
        state->snd_max = state->snd_nxt = state->iss;
        sendSynAck();
        startSynRexmitTimer();

        // TCP Fast Open: the data of our SYN stays in flight after the SYN-ACK (as
        // in Linux, which sends the SYN-ACK from the sequence number of the SYN)
        if (state->fastopenSynDataLen > 0) {
            state->snd_max = state->snd_nxt = state->iss + 1 + state->fastopenSynDataLen;
            emit(sndMaxSignal, state->snd_max);
        }

        // Note: code below is similar to processing SYN in LISTEN.

        // For consistency with that code, we ignore SYN+FIN here
        if (tcpHeader->getFinBit())
            EV_DETAIL << "SYN+FIN received: ignoring FIN\n";

        // The text of the crossing SYN is not acknowledged: this side is no Fast
        // Open server for it, and the peer sends it again after the handshake (as
        // Linux does). Before, it went into the receive queue at the sequence
        // number of the SYN, one byte early.
        if (B(tcpSegment->getByteLength()) > tcpHeader->getHeaderLength())
            EV_DETAIL << "Not acknowledging the text of the crossing SYN\n";

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
    else if (state->fastopenAccelerated && !isToBeAccepted()) {
        // the application has this TCP Fast Open connection
        sendIndicationToApp(TCP_I_CONNECTION_RESET);
    }

    // on RCV_RST the FSM goes to CLOSED for an active open, for a forked
    // connection, whose listener still exists, and for a TCP Fast Open connection;
    // only a passive open that did not fork returns to LISTEN
    return TCP_E_RCV_RST;
}

bool TcpConnection::processAckInEstabEtc(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader)
{
    EV_DETAIL << "Processing ACK in a data transfer state\n";

    int payloadLength = tcpSegment->getByteLength() - tcpHeader->getHeaderLength().get<B>();

    // ECN
    // (an AccECN connection uses the ECE bit for the ACE field, see processAccEcnAce())
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
    // RFC 8985 section 7.4.2: each ACK can end the episode of a tail loss probe
    if (seqLE(tcpHeader->getAckNo(), state->snd_max))
        tcpAlgorithm->processTlpAck(tcpHeader.get(), payloadLength);

    // rfc5961AckCheck: RFC 5961 section 5.2, which RFC 9293 allows (MAY-12): "the ACK value
    // is acceptable only if it is in the range of ((SND.UNA - MAX.SND.WND) =< SEG.ACK =<
    // SND.NXT). All incoming segments whose ACK value doesn't satisfy the above condition
    // MUST be discarded and an ACK sent back." MAX.SND.WND is max_window; an ACK above
    // SND.NXT takes the path of an ACK of unsent data below.
    if (state->rfc5961AckCheck && state->max_window > 0
        && seqLess(tcpHeader->getAckNo(), state->snd_una - state->max_window))
    {
        EV_INFO << "ACK below SND.UNA - MAX.SND.WND (RFC 5961 section 5.2): dropping the segment and sending an ACK\n";
        sendAck();
        return false; // means "drop"
    }

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
        // RFC 9293 section 3.10.7.4: "If SND.UNA =< SEG.ACK =< SND.NXT, the send window
        // should be updated", also by a duplicate ACK, by a window update when nothing is
        // in flight, and by a data segment of the peer; updateWndInfo() keeps old segments
        // out (SND.WL1, SND.WL2)
        if (state->snd_una == tcpHeader->getAckNo())
            updateWndInfo(tcpHeader);

        if (state->accEcnNegotiated)
            processAccEcnAce(tcpHeader);

        tcpAlgorithm->receivedAckForAlreadyAckedData(tcpHeader.get(), payloadLength);
    }
    else if (seqLE(tcpHeader->getAckNo(), state->snd_max)) {
        // ack in window.
        uint32_t old_snd_una = state->snd_una;
        state->snd_una = tcpHeader->getAckNo();

        emit(unackedSignal, state->snd_max - state->snd_una);

        // after retransmitting a lost segment, we may get an ack well ahead of snd_nxt
        if (seqLess(state->snd_nxt, state->snd_una))
            state->snd_nxt = state->snd_una;

        // mtuProbing: the peer acknowledged the whole probe (Linux tcp_clean_rtx_queue())
        if (state->mtupProbeSize != 0 && seqGE(state->snd_una, state->mtupProbeSeqEnd))
            mtuProbeSucceeded();

        // RFC 7323 Appendix D (fifth check the ACK field, ESTABLISHED STATE):
        // "If SND.UNA < SEG.ACK <= SND.NXT then, set SND.UNA <- SEG.ACK.
        // Also compute a new estimate of round-trip time.  If Snd.TS.OK
        // bit is on, use Snd.TSclock - SEG.TSecr; otherwise, use the
        // elapsed time since the first segment in the retransmission
        // queue was sent.  Any segments on the retransmission queue
        // that are thereby entirely acknowledged..."
        if (state->ts_enabled)
            tcpAlgorithm->rttMeasurementCompleteUsingTS(getTSecr(tcpHeader));
        // Note: If TS is disabled the RTT measurement is completed in TcpBaseAlg::receivedAckForUnackedData()

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

        // TCP_INFO time counters (busy_time): read-only bookkeeping -- if this ACK just
        // caught snd_una up to snd_max with nothing left queued either, the
        // connection has gone fully idle. See enqueueSendCommandData() for the
        // matching "became busy" entry.
        if (state->busyStartTime >= SIMTIME_ZERO && state->snd_una == state->snd_max
            && sendQueue->getBytesAvailable(state->snd_nxt) == 0)
        {
            state->busyTimeAccumulated += simTime() - state->busyStartTime;
            state->busyStartTime = -1;
        }
        updateSndbufLimitedChrono(); // an ACK of all data ends a limited interval

        // acked data no longer needed in rexmit queue
        uint32_t sackedBeforeDiscard = state->sack_enabled ? rexmitQueue->getTotalAmountOfSackedBytes() : 0;
        rexmitQueue->discardUpTo(discardUpToSeq);

        // A plain cumulative ACK carries no SACK option, so processSACKOption()
        // does not run to recompute the SACK scoreboard byte count. Refresh it
        // after the discard so the STATUS sackedBytes and caState reflect only what
        // is still SACKed above snd_una (Linux tp->sacked_out drops as snd_una
        // catches up); a full ACK that ends recovery must report 0, not the stale
        // pre-ACK count.
        if (state->sack_enabled)
            state->sackedBytes = rexmitQueue->getTotalAmountOfSackedBytes();

        // Delivered-bytes accounting (RFC 6937 DeliveredData, RFC 8985): the change of
        // snd_una plus the signed change of the SACKed bytes. processSACKOption() counts
        // the newly SACKed bytes; here count the newly acknowledged bytes, less those
        // that a SACK reported before, so that each byte counts once (Linux counts a
        // segment once, at its first SACK or ACK).
        if (seqGreater(discardUpToSeq, old_snd_una)) {
            uint32_t sackedCovered = state->sack_enabled ? sackedBeforeDiscard - state->sackedBytes : 0;
            state->deliveredBytes += (discardUpToSeq - old_snd_una) - sackedCovered;
            emit(deliveredSignal, (unsigned long)state->deliveredBytes);
        }

        // AccECN, before the algorithm processes the ACK, so that a CE mark acts as an
        // ECN-Echo of this ACK. RFC 9768 section 3.2.2.1: "When an AccECN Server in
        // SYN-RCVD state receives a pure ACK with SYN=0 and no SACK blocks, it MUST
        // infer the meaning of each possible value of the ACE field from Table 4": 0b110
        // reports a CE-marked SYN-ACK (s.cep = 6), and 0b000 a bleached handshake, after
        // which the server "MUST NOT set ECT on outgoing packets and MUST NOT respond to
        // AccECN feedback". Each later ACK carries the CE count.
        if (state->accEcnNegotiated) {
            if (fsm.getState() == TCP_S_SYN_RCVD) {
                bool sackBlocks = false;
                for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++)
                    if (tcpHeader->getHeaderOption(i)->getKind() == TCPOPTION_SACK)
                        sackBlocks = true;
                uint8_t ace = (tcpHeader->getAeBit() ? 4 : 0) | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0);
                if (payloadLength == 0 && !sackBlocks && ace == 6) {
                    // section 3.2.2.2: the server "then triggers the congestion control
                    // actions necessary to respond to the CE feedback" (Linux only counts
                    // it); the algorithm does it at the first ACK of data
                    state->deliveredCePkts = 1;
                    emit(deliveredCeSignal, (unsigned long)state->deliveredCePkts);
                    state->gotEce = true;
                    EV_INFO << "AccECN: the ACK of the SYN-ACK reports a CE-marked SYN-ACK\n";
                }
                else if (payloadLength == 0 && !sackBlocks && ace == 0) {
                    state->accEcnNoEct = true;
                    state->accEcnIgnoreFeedback = true;
                    state->ect = false;
                    EV_INFO << "AccECN: the ACK of the SYN-ACK has ACE 0, no ECT and no response to the feedback\n";
                }
            }
            else
                processAccEcnAce(tcpHeader);
        }

        updateWndInfo(tcpHeader);

        // if segment contains data, wait until data has been forwarded to app before sending ACK,
        // otherwise we would use an old ACKNo
        // The ACK that completes the handshake in SYN_RCVD acknowledges only the
        // SYN-ACK, and established() has just initialized the algorithm. But a TCP
        // Fast Open server can have sent data in SYN_RCVD: an ACK beyond the SYN-ACK
        // acknowledges data, and the algorithm processes it.
        bool acksFastOpenData = state->fastopenAccelerated && seqGreater(tcpHeader->getAckNo(), state->iss + 1);
        if (payloadLength == 0 && (fsm.getState() != TCP_S_SYN_RCVD || acksFastOpenData)) {
            // notify
            tcpAlgorithm->receivedAckForUnackedData(old_snd_una);

            // in the receivedAckForUnackedData we need the old value
            state->dupacks = 0;
            state->limitedTransmitBytes = 0;

            emit(dupAcksSignal, state->dupacks);
        }
    }
    else {
        ASSERT(seqGreater(tcpHeader->getAckNo(), state->snd_max)); // from if-ladder

        // send an ACK, drop the segment, and return.
        tcpAlgorithm->receivedAckForUnsentData(tcpHeader->getAckNo());
        state->dupacks = 0;
        state->limitedTransmitBytes = 0;

        emit(dupAcksSignal, state->dupacks);

        return false; // means "drop"
    }

    return true;
}

void TcpConnection::countAccEcnReceived(Packet *tcpSegment, int payloadLength)
{
    // section 3.2: r.cep counts the CE-marked packets, and r.e0b, r.ceb and r.e1b the
    // payload bytes of each codepoint (section 3.2.1), for the ACE field and the option
    int ecn = receivedEcnCodepoint(tcpSegment);
    if (ecn == IP_ECN_CE)
        state->rcvCePkts++;
    if (payloadLength > 0) {
        if (ecn == IP_ECN_ECT_0)
            state->rcvEct0Bytes += payloadLength;
        else if (ecn == IP_ECN_CE)
            state->rcvCeBytes += payloadLength;
        else if (ecn == IP_ECN_ECT_1)
            state->rcvEct1Bytes += payloadLength;
    }
}

void TcpConnection::processAccEcnAce(const Ptr<const TcpHeader>& tcpHeader)
{
    // Like Linux (__tcp_accecn_process()), only an ACK that makes progress (newly
    // acknowledged or SACKed data, or a newer timestamp) counts; the counter of a later
    // one covers the CE marks of a duplicate ACK before it.
    if (state->deliveredBytes == state->prrDeliveredMark && !state->accEcnTsProgress)
        return;

    uint8_t ace = (tcpHeader->getAeBit() ? 4 : 0) | (tcpHeader->getCwrBit() ? 2 : 0) | (tcpHeader->getEceBit() ? 1 : 0);
    uint64_t deliveredBytesThisAck = state->deliveredBytes - state->prrDeliveredMark;
    uint32_t mss = state->snd_mss > 0 ? state->snd_mss : 1;
    uint32_t deliveredPkts = (uint32_t)((deliveredBytesThisAck + mss - 1) / mss);

    // The AccECN option (RFC 9768 section 3.2.3.1, Appendix A.1): each field gives the
    // 24 least significant bits of a byte counter of the receiver, and the increment
    // is the difference to the local counter, mod 2^24 (the local counters here are
    // less the initial values of section 3.2.1: 1, 0 and 1).
    bool cebValid = false;
    uint32_t dCeb = 0;
    for (size_t i = 0; i < tcpHeader->getHeaderOptionArraySize() && !state->accEcnOptionIgnored; i++) {
        auto accEcnOption = dynamic_cast<const TcpOptionAccEcn *>(tcpHeader->getHeaderOption(i));
        if (accEcnOption == nullptr)
            continue;
        int numFields = std::min(3, (accEcnOption->getLength() - 2) / 3);
        bool order0 = accEcnOption->getKind() == TCPOPTION_ACCECN0;
        if (numFields >= (order0 ? 1 : 3))
            state->deliveredEct0Bytes += ((accEcnOption->getEct0Bytes() - 1) - state->deliveredEct0Bytes) & 0xFFFFFF;
        if (numFields >= 2) {
            dCeb = (accEcnOption->getCeBytes() - state->deliveredCeBytes) & 0xFFFFFF;
            state->deliveredCeBytes += dCeb;
            state->accEcnCebSeen = cebValid = true;
        }
        if (numFields >= (order0 ? 3 : 1))
            state->deliveredEct1Bytes += ((accEcnOption->getEct1Bytes() - 1) - state->deliveredEct1Bytes) & 0xFFFFFF;
        EV_INFO << "AccECN option: delivered bytes ECT(0)=" << state->deliveredEct0Bytes << ", CE=" << state->deliveredCeBytes
                << ", ECT(1)=" << state->deliveredEct1Bytes << "\n";
    }

    // RFC 9768 section 3.2.2.5.2 and Appendix A.2: the ACE field counts mod 8 (s.cep
    // starts at 5). An ACK of more than 7 packets could hide a full cycle of the counter,
    // and the sender "SHOULD deem whether it cycled by taking the safest likely case
    // under the prevailing conditions": the most CE marks that the packets allow
    // (dSafer.cep). As Linux (__tcp_accecn_process()), the CE bytes of the option
    // decide it (Appendix A.2.2: d.cep is safe enough if the marked segments would
    // otherwise be smaller than half an MSS); without them, large ACKs that are usual
    // for the connection (an average of more than 4 packets, pkts_acked_ewma) show
    // ACK compression, not a cycle.
    int delta = ((int)ace - 5 - (int)(state->deliveredCePkts & 7)) & 7;
    if (deliveredPkts > 0) {
        if (state->pktsAckedEwma == 0)
            state->pktsAckedEwma = deliveredPkts << 6;
        else
            state->pktsAckedEwma = std::min<uint32_t>((((state->pktsAckedEwma << 6) - state->pktsAckedEwma) + (deliveredPkts << 6)) >> 6, 0xFFFF);
    }
    int resolvedDelta = delta;
    if (deliveredPkts > 7) {
        int safeDelta = (int)deliveredPkts - (((int)deliveredPkts - delta) & 7);
        if (cebValid) {
            if (dCeb == 0)
                resolvedDelta = delta;
            else if (deliveredPkts >= 16 && (state->sack_enabled || !state->lossRecovery)) {
                // Linux: the CE packets that the CE bytes give at the average segment size
                if (deliveredBytesThisAck <= dCeb)
                    resolvedDelta = safeDelta;
                else {
                    uint32_t estimate = (uint32_t)(((uint64_t)dCeb * deliveredPkts + deliveredBytesThisAck - 1) / deliveredBytesThisAck);
                    resolvedDelta = std::min(safeDelta, delta + (int)(estimate & ~7u));
                }
            }
            else if (dCeb > (uint32_t)delta * mss)
                resolvedDelta = safeDelta;
            else if (dCeb < ((uint32_t)safeDelta * mss) >> 1)
                resolvedDelta = delta;
            else
                resolvedDelta = safeDelta;
        }
        else if (state->pktsAckedEwma <= (4u << 6))
            resolvedDelta = safeDelta;
    }

    EV_INFO << "AccECN: ACE=" << (int)ace << ", packets acknowledged=" << deliveredPkts << ", new CE marks="
            << resolvedDelta << " (mod 8: " << delta << ")\n";
    if (resolvedDelta > 0) {
        state->deliveredCePkts += resolvedDelta;
        emit(deliveredCeSignal, (unsigned long)state->deliveredCePkts);
        // RFC 9768 section 3.1.5: a host in AccECN mode "is obliged to respond
        // appropriately to AccECN feedback", as RFC 3168 section 6.1 defines for an
        // ECN-Echo (section 4: the response follows an increase of s.cep)
        if (!state->accEcnIgnoreFeedback)
            state->gotEce = true;
        else
            EV_INFO << "AccECN: no response to the feedback, because the ACK of the SYN-ACK had ACE 0\n";
    }
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
    state->syn_rexmit_timeout = tcpMain->par("initialRto"); // RFC 6298 section 2.1, also for the SYN (section 5.7)
    rescheduleAfter(state->syn_rexmit_timeout, synRexmitTimer);
}

void TcpConnection::process_TIMEOUT_SYN_REXMIT(TcpEventCode& event)
{
    // synRetries: Linux net.ipv4.tcp_syn_retries, also for the SYN-ACK (INET has one
    // value for both); the retransmissions alone limit the setup (RFC 9293 MUST-23)
    if (++state->syn_rexmit_count > state->synRetries) {
        EV_INFO << "Retransmission count during connection setup exceeds " << state->synRetries << ", giving up\n";
        // the setup times out: an active open tells the application (TCP_I_TIMED_OUT, as
        // ETIMEDOUT of Linux), and a passive open returns to LISTEN, or closes if it was
        // forked (RFC 9293 section 3.10.7.3)
        event = TCP_E_TIMEOUT_CONN_ESTAB;
        process_TIMEOUT_CONN_ESTAB();
        return;
    }

    // TCP Fast Open: the third transmission of a SYN with a Fast Open option or
    // data suggests a path that drops them (RFC 7413 section 4.1.3.1; Linux
    // tcp_fastopen_active_detect_blackhole())
    if (fsm.getState() == TCP_S_SYN_SENT && (state->fastopenSynCarriedOption || state->fastopenSynDataLen > 0)
        && state->syn_rexmit_count == TFO_BLACKHOLE_REXMIT_COUNT)
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

    // reschedule timer: RFC 6298 section 5.5 doubles it at each expiry. With
    // synLinearTimeouts, as Linux net.ipv4.tcp_syn_linear_timeouts, the first
    // retransmissions of a SYN keep the initial timeout. maxRexmitTimeout limits it,
    // as the RTO of data (section 2.5).
    if (!(fsm.getState() == TCP_S_SYN_SENT && state->syn_rexmit_count <= state->synLinearTimeouts))
        state->syn_rexmit_timeout *= 2;
    simtime_t maxRexmitTimeout = tcpMain->par("maxRexmitTimeout");
    if (state->syn_rexmit_timeout > maxRexmitTimeout)
        state->syn_rexmit_timeout = maxRexmitTimeout;

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

