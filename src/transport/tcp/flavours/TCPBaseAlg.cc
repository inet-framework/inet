//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2010 Thomas Reschka
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "TCPBaseAlg.h"
#include "TCP.h"
#include "TCPSACKRexmitQueue.h"


//
// Some constants below. MIN_REXMIT_TIMEOUT is the minimum allowed retransmit
// interval.  It is currently one second but e.g. a FreeBSD kernel comment says
// it "will ultimately be reduced to 3 ticks for algorithmic stability,
// leaving the 200ms variance to deal with delayed-acks, protocol overheads.
// A 1 second minimum badly breaks throughput on any network faster then
// a modem that has minor but continuous packet loss unrelated to congestion,
// such as on a wireless network."
//
// RFC 1122, page 95:
// "A TCP SHOULD implement a delayed ACK, but an ACK should not
// be excessively delayed; in particular, the delay MUST be
// less than 0.5 seconds, and in a stream of full-sized
// segments there SHOULD be an ACK for at least every second
// segment."

#define DELAYED_ACK_TIMEOUT   0.2   // 200ms (RFC 1122: MUST be less than 0.5 seconds)
#define MAX_REXMIT_COUNT       12   // 12 retries
#define MIN_REXMIT_TIMEOUT    1.0   // 1s
//#define MIN_REXMIT_TIMEOUT    0.6   // 600ms (3 ticks)
#define MAX_REXMIT_TIMEOUT    240   // 2 * MSL (RFC 1122)
#define MIN_PERSIST_TIMEOUT     5   //  5s
#define MAX_PERSIST_TIMEOUT    60   // 60s

TCPBaseAlgStateVariables::TCPBaseAlgStateVariables()
{
    rexmit_count = 0;
    rexmit_timeout = 3.0;

    persist_factor = 0;
    persist_timeout = 5.0;

    snd_cwnd = 0; // will be set to SMSS when connection is established

    rtseq = 0;
    rtseq_sendtime = 0;

    // Jacobson's alg: srtt must be initialized to 0, rttvar to a value which
    // will yield rto = 3s initially.
    srtt = 0;
    rttvar = 3.0 / 4.0;

    numRtos = 0;

    // RFC 3782, page 4:
    // "The algorithm specified in this document uses a variable "recover",
    // whose initial value is the initial send sequence number."
    recover = iss;
    firstPartialACK = false;
}

std::string TCPBaseAlgStateVariables::info() const
{
    std::stringstream out;
    out << TCPStateVariables::info();
    out << " snd_cwnd=" << snd_cwnd;
    out << " rto=" << rexmit_timeout;
    return out.str();
}

std::string TCPBaseAlgStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TCPStateVariables::detailedInfo();
    out << "snd_cwnd=" << snd_cwnd << "\n";
    out << "rto=" << rexmit_timeout << "\n";
    out << "persist_timeout=" << persist_timeout << "\n";
    // TBD add others too
    return out.str();
}

TCPBaseAlg::TCPBaseAlg() : TCPAlgorithm(),
  state((TCPBaseAlgStateVariables *&)TCPAlgorithm::state)
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = NULL;
    cwndVector = ssthreshVector = rttVector = srttVector = rttvarVector = rtoVector = numRtosVector = NULL;
}

TCPBaseAlg::~TCPBaseAlg()
{
    // Note: don't delete "state" here, it'll be deleted from TCPConnection

    // cancel and delete timers
    if (rexmitTimer)     delete cancelEvent(rexmitTimer);
    if (persistTimer)    delete cancelEvent(persistTimer);
    if (delayedAckTimer) delete cancelEvent(delayedAckTimer);
    if (keepAliveTimer)  delete cancelEvent(keepAliveTimer);

    // delete statistics objects
    delete cwndVector;
    delete ssthreshVector;
    delete rttVector;
    delete srttVector;
    delete rttvarVector;
    delete rtoVector;
    delete numRtosVector;
}

void TCPBaseAlg::initialize()
{
    TCPAlgorithm::initialize();

    rexmitTimer = new cMessage("REXMIT");
    persistTimer = new cMessage("PERSIST");
    delayedAckTimer = new cMessage("DELAYEDACK");
    keepAliveTimer = new cMessage("KEEPALIVE");

    rexmitTimer->setContextPointer(conn);
    persistTimer->setContextPointer(conn);
    delayedAckTimer->setContextPointer(conn);
    keepAliveTimer->setContextPointer(conn);

    if (conn->getTcpMain()->recordStatistics)
    {
        cwndVector = new cOutVector("cwnd");
        ssthreshVector = new cOutVector("ssthresh");
        rttVector = new cOutVector("measured RTT");
        srttVector = new cOutVector("smoothed RTT");
        rttvarVector = new cOutVector("RTTVAR");
        rtoVector = new cOutVector("RTO");
        numRtosVector = new cOutVector("numRTOs");
    }
}

void TCPBaseAlg::established(bool active)
{
    // initialize cwnd (we may learn SMSS during connection setup)

    // RFC 3390, page 2: "The upper bound for the initial window is given more precisely in
    // (1):
    //
    //   min (4*MSS, max (2*MSS, 4380 bytes))                        (1)
    //
    // Note: Sending a 1500 byte packet indicates a maximum segment size
    // (MSS) of 1460 bytes (assuming no IP or TCP options).  Therefore,
    // limiting the initial window's MSS to 4380 bytes allows the sender to
    // transmit three segments initially in the common case when using 1500
    // byte packets.
    //
    // Equivalently, the upper bound for the initial window size is based on
    // the MSS, as follows:
    //
    //   If (MSS <= 1095 bytes)
    //     then win <= 4 * MSS;
    //   If (1095 bytes < MSS < 2190 bytes)
    //     then win <= 4380;
    //   If (2190 bytes <= MSS)
    //     then win <= 2 * MSS;
    //
    // This increased initial window is optional: a TCP MAY start with a
    // larger initial window.  However, we expect that most general-purpose
    // TCP implementations would choose to use the larger initial congestion
    // window given in equation (1) above.
    //
    // This upper bound for the initial window size represents a change from
    // RFC 2581 [RFC2581], which specified that the congestion window be
    // initialized to one or two segments.
    // (...)
    // If the SYN or SYN/ACK is
    // lost, the initial window used by a sender after a correctly
    // transmitted SYN MUST be one segment consisting of MSS bytes."
    if (state->increased_IW_enabled && state->syn_rexmit_count == 0)
    {
        state->snd_cwnd = std::min(4 * state->snd_mss, std::max(2 * state->snd_mss, (uint32)4380));
        tcpEV << "Enabled Increased Initial Window, CWND is set to " << state->snd_cwnd << "\n";
    }
    // RFC 2001, page 3:
    // " 1.  Initialization for a given connection sets cwnd to one segment
    // and ssthresh to 65535 bytes."
    else
        state->snd_cwnd = state->snd_mss; // RFC 2001

    if (active)
    {
        // finish connection setup with ACK (possibly piggybacked on data)
        tcpEV << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData()) // FIXME TODO - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
            conn->sendAck();
    }
}

void TCPBaseAlg::connectionClosed()
{
    cancelEvent(rexmitTimer);
    cancelEvent(persistTimer);
    cancelEvent(delayedAckTimer);
    cancelEvent(keepAliveTimer);
}

void TCPBaseAlg::processTimer(cMessage *timer, TCPEventCode& event)
{
    if (timer == rexmitTimer)
        processRexmitTimer(event);
    else if (timer == persistTimer)
        processPersistTimer(event);
    else if (timer == delayedAckTimer)
        processDelayedAckTimer(event);
    else if (timer == keepAliveTimer)
        processKeepAliveTimer(event);
    else
        throw cRuntimeError(timer, "unrecognized timer");
}

void TCPBaseAlg::processRexmitTimer(TCPEventCode& event)
{
    tcpEV << "TCB: " << state->info() << "\n";

    //"
    // For any state if the retransmission timeout expires on a segment in
    // the retransmission queue, send the segment at the front of the
    // retransmission queue again, reinitialize the retransmission timer,
    // and return.
    //"
    // Also: abort connection after max 12 retries.
    //
    // However, retransmission is actually more complicated than that
    // in RFC 793 above, we'll leave it to subclasses (e.g. TCPTahoe, TCPReno).
    //
    if (++state->rexmit_count > MAX_REXMIT_COUNT)
    {
        tcpEV << "Retransmission count exceeds " << MAX_REXMIT_COUNT << ", aborting connection\n";
        conn->signalConnectionTimeout();
        event = TCP_E_ABORT;  // TBD maybe rather introduce a TCP_E_TIMEDOUT event
        return;
    }

    tcpEV << "Performing retransmission #" << state->rexmit_count
          << "; increasing RTO from " << state->rexmit_timeout << "s ";

    //
    // Karn's algorithm is implemented below:
    //  (1) don't measure RTT for retransmitted packets.
    //  (2) RTO should be doubled after retransmission ("exponential back-off")
    //

    // restart the retransmission timer with twice the latest RTO value, or with the max, whichever is smaller
    state->rexmit_timeout += state->rexmit_timeout;
    if (state->rexmit_timeout > MAX_REXMIT_TIMEOUT)
        state->rexmit_timeout = MAX_REXMIT_TIMEOUT;

    conn->scheduleTimeout(rexmitTimer, state->rexmit_timeout);

    tcpEV << " to " << state->rexmit_timeout << "s, and cancelling RTT measurement\n";

    // cancel round-trip time measurement
    state->rtseq_sendtime = 0;

    state->numRtos++;

    if (numRtosVector)
        numRtosVector->record(state->numRtos);

    // if sacked_enabled reset sack related flags
    if (state->sack_enabled)
    {
        conn->rexmitQueue->resetSackedBit();
        conn->rexmitQueue->resetRexmittedBit();

        // RFC 3517, page 8: "If an RTO occurs during loss recovery as specified in this document,
        // RecoveryPoint MUST be set to HighData.  Further, the new value of
        // RecoveryPoint MUST be preserved and the loss recovery algorithm
        // outlined in this document MUST be terminated.  In addition, a new
        // recovery phase (as described in section 5) MUST NOT be initiated
        // until HighACK is greater than or equal to the new value of
        // RecoveryPoint."
        if (state->lossRecovery)
        {
            state->recoveryPoint = state->snd_max; // HighData = snd_max
            tcpEV << "Loss Recovery terminated.\n";
            state->lossRecovery = false;
        }
    }

    state->time_last_data_sent = simTime();

    //
    // Leave congestion window management and actual retransmission to
    // subclasses (e.g. TCPTahoe, TCPReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and do the retransmission as they like.
    //
}

void TCPBaseAlg::processPersistTimer(TCPEventCode& event)
{
    // setup and restart the PERSIST timer
    // FIXME Calculation of PERSIST timer is not as simple as done here!
    // It depends on RTT calculations and is bounded to 5-60 seconds.
    // This simplified PERSIST timer calculation generates values
    // as presented in [Stevens, W.R.: TCP/IP Illustrated, Volume 1, chapter 22.2]
    // (5, 5, 6, 12, 24, 48, 60, 60, 60...)
    if (state->persist_factor == 0)
        state->persist_factor++;
    else if (state->persist_factor < 64)
        state->persist_factor = state->persist_factor * 2;

    state->persist_timeout = state->persist_factor * 1.5; // 1.5 is a factor for typical LAN connection [Stevens, W.R.: TCP/IP Ill. Vol. 1, chapter 22.2]

    // PERSIST timer is bounded to 5-60 seconds
    if (state->persist_timeout < MIN_PERSIST_TIMEOUT)
        state->rexmit_timeout = MIN_PERSIST_TIMEOUT;

    if (state->persist_timeout > MAX_PERSIST_TIMEOUT)
        state->rexmit_timeout = MAX_PERSIST_TIMEOUT;

    conn->scheduleTimeout(persistTimer, state->persist_timeout);

    // sending persist probe
    conn->sendProbe();
}

void TCPBaseAlg::processDelayedAckTimer(TCPEventCode& event)
{
    state->ack_now = true;
    conn->sendAck();
}

void TCPBaseAlg::processKeepAliveTimer(TCPEventCode& event)
{
    // TBD
    // RFC 1122, page 102:
    // "A "keep-alive" mechanism periodically probes the other
    // end of a connection when the connection is otherwise
    // idle, even when there is no data to be sent.  The TCP
    // specification does not include a keep-alive mechanism
    // because it could:  (1) cause perfectly good connections
    // to break during transient Internet failures; (2)
    // consume unnecessary bandwidth ("if no one is using the
    // connection, who cares if it is still good?"); and (3)
    // cost money for an Internet path that charges for
    // packets."
}

void TCPBaseAlg::startRexmitTimer()
{
    // start counting retransmissions for this seq number.
    // Note: state->rexmit_timeout is set from rttMeasurementComplete().
    state->rexmit_count = 0;

    // schedule timer
    conn->scheduleTimeout(rexmitTimer, state->rexmit_timeout);
}

void TCPBaseAlg::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // update smoothed RTT estimate (srtt) and variance (rttvar)
    const double g = 0.125; // 1 / 8; (1 - alpha) where alpha == 7 / 8;
    simtime_t newRTT = tAcked - tSent;

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    simtime_t err = newRTT - srtt;

    srtt += g * err;
    rttvar += g * (fabs(err) - rttvar);

    // assign RTO (here: rexmit_timeout) a new value
    simtime_t rto = srtt + 4 * rttvar;

    if (rto > MAX_REXMIT_TIMEOUT)
        rto = MAX_REXMIT_TIMEOUT;
    else if (rto < MIN_REXMIT_TIMEOUT)
        rto = MIN_REXMIT_TIMEOUT;

    state->rexmit_timeout = rto;

    // record statistics
    tcpEV << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (srtt * 1000)
          << "ms, new RTO=" << (rto * 1000) << "ms\n";

    if (rttVector)
        rttVector->record(newRTT);

    if (srttVector)
        srttVector->record(srtt);

    if (rttvarVector)
        rttvarVector->record(rttvar);

    if (rtoVector)
        rtoVector->record(rto);
}

void TCPBaseAlg::rttMeasurementCompleteUsingTS(uint32 echoedTS)
{
    ASSERT(state->ts_enabled);

    // Note: The TS option is using uint32 values (ms precision) therefore we convert the current simTime also to a uint32 value (ms precision)
    // and then convert back to simtime_t to use rttMeasurementComplete() to update srtt and rttvar
    uint32 now = conn->convertSimtimeToTS(simTime());
    simtime_t tSent = conn->convertTSToSimtime(echoedTS);
    simtime_t tAcked = conn->convertTSToSimtime(now);
    rttMeasurementComplete(tSent, tAcked);
}

bool TCPBaseAlg::sendData()
{
    //
    // Nagle's algorithm: when a TCP connection has outstanding data that has not
    // yet been acknowledged, small segments cannot be sent until the outstanding
    // data is acknowledged. (In this case, small amounts of data are collected
    // by TCP and sent in a single segment.)
    //
    // FIXME there's also something like this: can still send if
    // "b) a segment that can be sent is at least half the size of
    // the largest window ever advertised by the receiver"

    bool fullSegmentsOnly = state->nagle_enabled && state->snd_una != state->snd_max;

    if (fullSegmentsOnly)
        tcpEV << "Nagle is enabled and there's unacked data: only full segments will be sent\n";

    // RFC 2581, pages 7 and 8: "When TCP has not received a segment for
    // more than one retransmission timeout, cwnd is reduced to the value
    // of the restart window (RW) before transmission begins.
    // For the purposes of this standard, we define RW = IW.
    // (...)
    // Using the last time a segment was received to determine whether or
    // not to decrease cwnd fails to deflate cwnd in the common case of
    // persistent HTTP connections [HTH98].
    // (...)
    // Therefore, a TCP SHOULD set cwnd to no more than RW before beginning
    // transmission if the TCP has not sent data in an interval exceeding
    // the retransmission timeout."
    if (!conn->isSendQueueEmpty())  // do we have any data to send?
    {
        if ((simTime() - state->time_last_data_sent) > state->rexmit_timeout)
        {
            // RFC 5681, page 11: "For the purposes of this standard, we define RW = min(IW,cwnd)."
            if (state->increased_IW_enabled)
                state->snd_cwnd = std::min(std::min(4 * state->snd_mss, std::max(2 * state->snd_mss, (uint32)4380)), state->snd_cwnd);
            else
                state->snd_cwnd = state->snd_mss;

            tcpEV << "Restarting idle connection, CWND is set to " << state->snd_cwnd << "\n";
        }
    }

    //
    // Send window is effectively the minimum of the congestion window (cwnd)
    // and the advertised window (snd_wnd).
    //
    return conn->sendData(fullSegmentsOnly, state->snd_cwnd);
}

void TCPBaseAlg::sendCommandInvoked()
{
    // try sending
    sendData();
}

void TCPBaseAlg::receivedOutOfOrderSegment()
{
    state->ack_now = true;
    tcpEV << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void TCPBaseAlg::receiveSeqChanged()
{
    // If we send a data segment already (with the updated seqNo) there is no need to send an additional ACK
    if (state->full_sized_segment_counter == 0 && !state->ack_now && state->last_ack_sent == state->rcv_nxt && !delayedAckTimer->isScheduled()) // ackSent?
    {
        // tcpEV << "ACK has already been sent (possibly piggybacked on data)\n";
    }
    else
    {
        // RFC 2581, page 6:
        // "3.2 Fast Retransmit/Fast Recovery
        // (...)
        // In addition, a TCP receiver SHOULD send an immediate ACK
        // when the incoming segment fills in all or part of a gap in the
        // sequence space."
        if (state->lossRecovery)
            state->ack_now = true; // although not mentioned in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861] seems like we have to set ack_now

        if (!state->delayed_acks_enabled) // delayed ACK disabled
        {
            tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK disabled) sending ACK now\n";
            conn->sendAck();
        }
        else // delayed ACK enabled
        {
            if (state->ack_now)
            {
                tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but ack_now is set) sending ACK now\n";
                conn->sendAck();
            }
            // RFC 1122, page 96: "in a stream of full-sized segments there SHOULD be an ACK for at least every second segment."
            else if (state->full_sized_segment_counter >= 2)
            {
                tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled, but full_sized_segment_counter=" << state->full_sized_segment_counter << ") sending ACK now\n";
                conn->sendAck();
            }
            else
            {
                tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", (delayed ACK enabled and full_sized_segment_counter=" << state->full_sized_segment_counter << ") scheduling ACK\n";
                if (!delayedAckTimer->isScheduled()) // schedule delayed ACK timer if not already running
                    conn->scheduleTimeout(delayedAckTimer, DELAYED_ACK_TIMEOUT);
            }
        }
    }
}

void TCPBaseAlg::receivedDataAck(uint32 firstSeqAcked)
{
    if (!state->ts_enabled)
    {
        // if round-trip time measurement is running, check if rtseq has been acked
        if (state->rtseq_sendtime != 0 && seqLess(state->rtseq, state->snd_una))
        {
            // print value
            tcpEV << "Round-trip time measured on rtseq=" << state->rtseq << ": "
                  << floor((simTime() - state->rtseq_sendtime) * 1000 + 0.5) << "ms\n";

            rttMeasurementComplete(state->rtseq_sendtime, simTime()); // update RTT variables with new value

            // measurement finished
            state->rtseq_sendtime = 0;
        }
    }

    //
    // handling of retransmission timer: if the ACK is for the last segment sent
    // (no data in flight), cancel the timer, otherwise restart the timer
    // with the current RTO value.
    //
    if (state->snd_una == state->snd_max)
    {
        if (rexmitTimer->isScheduled())
        {
            tcpEV << "ACK acks all outstanding segments, cancel REXMIT timer\n";
            cancelEvent(rexmitTimer);
        }
        else
            tcpEV << "There were no outstanding segments, nothing new in this ACK.\n";
    }
    else
    {
        tcpEV << "ACK acks some but not all outstanding segments ("
              << (state->snd_max - state->snd_una) << " bytes outstanding), "
              << "restarting REXMIT timer\n";
        cancelEvent(rexmitTimer);
        startRexmitTimer();
    }

    //
    // handling of PERSIST timer:
    // If data sender received a zero-sized window, check retransmission timer.
    //  If retransmission timer is not scheduled, start PERSIST timer if not already
    //  running.
    //
    // If data sender received a non zero-sized window, check PERSIST timer.
    //  If PERSIST timer is scheduled, cancel PERSIST timer.
    //
    if (state->snd_wnd == 0) // received zero-sized window?
    {
        if (rexmitTimer->isScheduled())
        {
            if (persistTimer->isScheduled())
            {
                tcpEV << "Received zero-sized window and REXMIT timer is running therefore PERSIST timer is canceled.\n";
                cancelEvent(persistTimer);
                state->persist_factor = 0;
            }
            else
                tcpEV << "Received zero-sized window and REXMIT timer is running therefore PERSIST timer is not started.\n";
        }
        else
        {
            if (!persistTimer->isScheduled())
            {
                tcpEV << "Received zero-sized window therefore PERSIST timer is started.\n";
                conn->scheduleTimeout(persistTimer, state->persist_timeout);
            }
            else
                tcpEV << "Received zero-sized window and PERSIST timer is already running.\n";
        }
    }
    else // received non zero-sized window?
    {
        if (persistTimer->isScheduled())
        {
            tcpEV << "Received non zero-sized window therefore PERSIST timer is canceled.\n";
            cancelEvent(persistTimer);
            state->persist_factor = 0;
        }
    }

    //
    // Leave congestion window management and possible sending data to
    // subclasses (e.g. TCPTahoe, TCPReno).
    //
    // That is, subclasses will redefine this method, call us, then perform
    // window adjustments and send data (if there's room in the window).
    //
}

void TCPBaseAlg::receivedDuplicateAck()
{
    tcpEV << "Duplicate ACK #" << state->dupacks << "\n";

    bool fullSegmentsOnly = state->nagle_enabled && state->snd_una != state->snd_max;
    if (state->dupacks < DUPTHRESH && state->limited_transmit_enabled) // DUPTRESH = 3
        conn->sendOneNewSegment(fullSegmentsOnly, state->snd_cwnd); // RFC 3042

    //
    // Leave to subclasses (e.g. TCPTahoe, TCPReno) whatever they want to do
    // on duplicate Acks.
    //
    // That is, subclasses will redefine this method, call us, then perform
    // whatever action they want to do on dupAcks (e.g. retransmitting one segment).
    //
}

void TCPBaseAlg::receivedAckForDataNotYetSent(uint32 seq)
{
    // Note: In this case no immediate ACK will be send because not mentioned
    // in [Stevens, W.R.: TCP/IP Illustrated, Volume 2, page 861].
    // To force immediate ACK use:
        // state->ack_now = true;
        // tcpEV << "ACK acks something not yet sent, sending immediate ACK\n";
    tcpEV << "ACK acks something not yet sent, sending ACK\n";
    conn->sendAck();
}

void TCPBaseAlg::ackSent()
{
    state->full_sized_segment_counter = 0; // reset counter
    state->ack_now = false; // reset flag
    state->last_ack_sent = state->rcv_nxt; // update last_ack_sent, needed for TS option
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        cancelEvent(delayedAckTimer);
}

void TCPBaseAlg::dataSent(uint32 fromseq)
{
    // if retransmission timer not running, schedule it
    if (!rexmitTimer->isScheduled())
    {
        tcpEV << "Starting REXMIT timer\n";
        startRexmitTimer();
    }

    if (!state->ts_enabled)
    {
        // start round-trip time measurement (if not already running)
        if (state->rtseq_sendtime == 0)
        {
            // remember this sequence number and when it was sent
            state->rtseq = fromseq;
            state->rtseq_sendtime = simTime();
            tcpEV << "Starting rtt measurement on seq=" << state->rtseq << "\n";
        }
    }

    state->time_last_data_sent = simTime();
}

void TCPBaseAlg::restartRexmitTimer()
{
    if (rexmitTimer->isScheduled())
        cancelEvent(rexmitTimer);

    startRexmitTimer();
}
