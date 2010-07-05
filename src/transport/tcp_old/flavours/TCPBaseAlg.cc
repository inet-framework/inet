//
// Copyright (C) 2004 Andras Varga
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

#include "TCPBaseAlg_old.h"
#include "TCP_old.h"

using namespace tcp_old;

//
// Some constants below. MIN_REXMIT_TIMEOUT is the minimum allowed retransmit
// interval.  It is currently one second but e.g. a FreeBSD kernel comment says
// it "will ultimately be reduced to 3 ticks for algorithmic stability,
// leaving the 200ms variance to deal with delayed-acks, protocol overheads.
// A 1 second minimum badly breaks throughput on any network faster then
// a modem that has minor but continuous packet loss unrelated to congestion,
// such as on a wireless network."
//
#define DELAYED_ACK_TIMEOUT   0.2   // 200ms
#define MAX_REXMIT_COUNT       12   // 12 retries
#define MIN_REXMIT_TIMEOUT    1.0   // 1s
//#define MIN_REXMIT_TIMEOUT    0.6   // 600ms (3 ticks)
#define MAX_REXMIT_TIMEOUT    240   // 2*MSL (RFC1122)



TCPBaseAlgStateVariables::TCPBaseAlgStateVariables()
{
    // We disable delayed acks, since it appears that it isn't used in real-life TCPs.
    //
    // In SSFNet test suite http://www.ssfnet.org/Exchange/tcp/test/f5.html
    // the rule for delayed ACK is:
    //   An ACK must be sent immediatly when either of the following conditions exist:
    //    * Two full-sized packets received (to avoid too few ACKs).
    //    * Out of order packets received (to help trigger fast retransmission).
    //    * Received packet fills in all gap or part of gap of out of order data.
    // We do not implement this rule. In our measurements on network traffic, we
    // never encountered delayed ACKs.
    //
    delayed_acks_enabled = false;

    nagle_enabled = true; // FIXME this should be parameter eventually

    rexmit_count = 0;
    rexmit_timeout = 3.0;

    snd_cwnd = 0; // will be set to MSS when connection is established

    rtseq = 0;
    rtseq_sendtime = 0;

    // Jacobson's alg: srtt must be initialized to 0, rttvar to a value which
    // will yield rto = 3s initially.
    srtt = 0;
    rttvar = 3.0/4.0;
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
    out << "snd_cwnd = " << snd_cwnd << "\n";
    out << "rto = " << rexmit_timeout << "\n";
    // TBD add others too
    return out.str();
}

TCPBaseAlg::TCPBaseAlg() : TCPAlgorithm(),
  state((TCPBaseAlgStateVariables *&)TCPAlgorithm::state)
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = NULL;
    cwndVector = ssthreshVector = rttVector = srttVector = rttvarVector = rtoVector = NULL;
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
    }
}

void TCPBaseAlg::established(bool active)
{
    // initialize cwnd (we may learn MSS during connection setup --
    // this (MSS TCP option) is not implemented yet though)
    state->snd_cwnd = state->snd_mss;
    if (cwndVector) cwndVector->record(state->snd_cwnd);

    if (active)
    {
        // finish connection setup with ACK (possibly piggybacked on data)
        tcpEV << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData())
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
    if (timer==rexmitTimer)
        processRexmitTimer(event);
    else if (timer==persistTimer)
        processPersistTimer(event);
    else if (timer==delayedAckTimer)
        processDelayedAckTimer(event);
    else if (timer==keepAliveTimer)
        processKeepAliveTimer(event);
    else
        throw cRuntimeError(timer, "unrecognized timer");
}

void TCPBaseAlg::processRexmitTimer(TCPEventCode& event)
{
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
    // FIXME TBD finish (currently Persist Timer never gets scheduled)
    conn->sendProbe();
}

void TCPBaseAlg::processDelayedAckTimer(TCPEventCode& event)
{
    conn->sendAck();
}

void TCPBaseAlg::processKeepAliveTimer(TCPEventCode& event)
{
    // FIXME TBD
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
    const double g = 0.125;   // 1/8; (1-alpha) where alpha=7/8;
    simtime_t newRTT = tAcked-tSent;

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    simtime_t err = newRTT - srtt;

    srtt += g*err;
    rttvar += g*(fabs(err) - rttvar);

    // assign RTO (here: rexmit_timeout) a new value
    simtime_t rto = srtt + 4*rttvar;
    if (rto>MAX_REXMIT_TIMEOUT)
        rto = MAX_REXMIT_TIMEOUT;
    else if (rto<MIN_REXMIT_TIMEOUT)
        rto = MIN_REXMIT_TIMEOUT;

    state->rexmit_timeout = rto;

    // record statistics
    tcpEV << "Measured RTT=" << (newRTT*1000) << "ms, updated SRTT=" << (srtt*1000)
          << "ms, new RTO=" << (rto*1000) << "ms\n";
    if (rttVector) rttVector->record(newRTT);
    if (srttVector) srttVector->record(srtt);
    if (rttvarVector) rttvarVector->record(rttvar);
    if (rtoVector) rtoVector->record(rto);
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

    bool fullSegmentsOnly = state->nagle_enabled && state->snd_una!=state->snd_max;
    if (fullSegmentsOnly)
        tcpEV << "Nagle is enabled and there's unacked data: only full segments will be sent\n";

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
    tcpEV << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void TCPBaseAlg::receiveSeqChanged()
{
    // If we send a data segment already (with the updated seqNo) there is no need to send an additional ACK
    if (state->last_ack_sent == state->rcv_nxt && !delayedAckTimer->isScheduled()) // ackSent?
    {
        // tcpEV << "ACK has already been sent (possibly piggybacked on data)\n";
    }
    else
    {

        if (!state->delayed_acks_enabled)
        {
            tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", sending ACK now (delayed ACKs are disabled)\n";
            conn->sendAck();
        }
        else
        {
            // FIXME ACK should be generated for at least every second SMSS-sized segment!
            // schedule delayed ACK timer if not already running
            tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", scheduling ACK\n";
            if (!delayedAckTimer->isScheduled())
                conn->scheduleTimeout(delayedAckTimer, DELAYED_ACK_TIMEOUT);
        }
    }
}

void TCPBaseAlg::receivedDataAck(uint32 firstSeqAcked)
{
    // if round-trip time measurement is running, check if rtseq has been acked
    if (state->rtseq_sendtime!=0 && seqLess(state->rtseq, state->snd_una))
    {
        // print value
        tcpEV << "Round-trip time measured on rtseq=" << state->rtseq << ": "
              << floor((simTime() - state->rtseq_sendtime)*1000+0.5) << "ms\n";

        // update RTT variables with new value
        rttMeasurementComplete(state->rtseq_sendtime, simTime());

        // measurement finished
        state->rtseq_sendtime = 0;
    }

    //
    // handling of retransmission timer: if the ACK is for the last segment sent
    // (no data in flight), cancel the timer, otherwise restart the timer
    // with the current RTO value.
    //
    if (state->snd_una==state->snd_max)
    {
        if (rexmitTimer->isScheduled())
        {
            tcpEV << "ACK acks all outstanding segments, cancel REXMIT timer\n";
            cancelEvent(rexmitTimer);
        }
        else
        {
            tcpEV << "There were no outstanding segments, nothing new in this ACK.\n";
        }
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
    tcpEV << "ACK acks something not yet sent, sending immediate ACK\n";
    conn->sendAck();
}

void TCPBaseAlg::ackSent()
{
    state->last_ack_sent = state->rcv_nxt;
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

    // start round-trip time measurement (if not already running)
    if (state->rtseq_sendtime==0)
    {
        // remember this sequence number and when it was sent
        state->rtseq = fromseq;
        state->rtseq_sendtime = simTime();
        tcpEV << "Starting rtt measurement on seq=" << state->rtseq << "\n";
    }
}

void TCPBaseAlg::restartRexmitTimer()
{
    if (rexmitTimer->isScheduled())
        cancelEvent(rexmitTimer);
    startRexmitTimer();
}


