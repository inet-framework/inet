//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "PlainTCP.h"
#include "TCPMain.h"


#define DELAYED_ACK_TIMEOUT   0.2   // 200ms
#define MAX_REXMIT_COUNT       12   // 12 retries
#define MIN_REXMIT_TIMEOUT    0.5   // 500ms (1 "large tick") FIXME is this okay?
#define MAX_REXMIT_TIMEOUT    240   // 2*MSL (RFC1122)

Register_Class(PlainTCP);


PlainTCPStateVariables::PlainTCPStateVariables()
{
    delayed_acks_enabled = true;
    nagle_enabled = true;

    rexmit_seq = 0;
    rexmit_count = 0;
    rexmit_timeout = 3.0;

    snd_cwnd = 0; // will be set to MSS when connection is established
    ssthresh = 65535;

    dupacks = 0;

    rtseq = 0;
    t_rtseq_sent = 0;

    // Jacobson's alg: srtt must be initialized to 0, rttvar to a value which
    // will yield rto = 3s initially.
    srtt = 0;
    rttvar = 3.0/4.0;
}


PlainTCP::PlainTCP() : TCPAlgorithm()
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = NULL;
    state = NULL;
}

PlainTCP::~PlainTCP()
{
    // Note: don't delete "state" here, it'll be deleted from TCPConnection

    // Delete timers
    TCPMain *mod = conn->getTcpMain();
    delete mod->cancelEvent(rexmitTimer);
    delete mod->cancelEvent(persistTimer);
    delete mod->cancelEvent(delayedAckTimer);
    delete mod->cancelEvent(keepAliveTimer);
}

void PlainTCP::initialize()
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
}

TCPStateVariables *PlainTCP::createStateVariables()
{
    ASSERT(state==NULL);
    state = new PlainTCPStateVariables();
    return state;
}

void PlainTCP::established()
{
    state->snd_cwnd = state->snd_mss;
}

void PlainTCP::processTimer(cMessage *timer, TCPEventCode& event)
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
        throw new cException(timer, "unrecognized timer");
}

void PlainTCP::processRexmitTimer(TCPEventCode& event)
{
    //"
    // For any state if the retransmission timeout expires on a segment in
    // the retransmission queue, send the segment at the front of the
    // retransmission queue again, reinitialize the retransmission timer,
    // and return.
    //"
    //
    // Also, abort connection after max 12 retries
    //
    if (++state->rexmit_count > MAX_REXMIT_COUNT)
    {
        event = TCP_E_ABORT;  // TBD maybe rather introduce a TCP_E_BROKEN event
        return;
    }

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

    // cancel round-trip time measurement
    state->t_rtseq_sent = 0;

    //
    // Congestion control, etc:
    //
    int flight_size = state->snd_max - state->snd_una; // FIXME ok? ssf says: flight_size = snd_wnd = min(cnwd, rwnd) ???
    state->ssthresh = Max(flight_size/2, 2*state->snd_mss);
    state->snd_cwnd = state->snd_mss;

    // retransmit
    conn->retransmitData();
}

void PlainTCP::processPersistTimer(TCPEventCode& event)
{
    // FIXME TBD
}

void PlainTCP::processDelayedAckTimer(TCPEventCode& event)
{
    conn->sendAck();
}

void PlainTCP::processKeepAliveTimer(TCPEventCode& event)
{
    // FIXME TBD
}

void PlainTCP::startRexmitTimer()
{
    // start counting retransmissions for this seq number.
    // Note: state->rexmit_timeout is set from rttMeasurementComplete().
    state->rexmit_seq = state->snd_una;
    state->rexmit_count = 0;

    // schedule timer
    conn->scheduleTimeout(rexmitTimer, state->rexmit_timeout);
}

void PlainTCP::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // update smoothed RTT estimate (srtt) and variance (rttvar)
    const double g = 0.125;   // 1/8; (1-alpha) if alpha = 7/8;
    double newRTT = tAcked-tSent;

    double& srtt = state->srtt;
    double& rttvar = state->rttvar;

    double err = srtt - newRTT;

    srtt += g*err;
    rttvar += g*(fabs(err) - rttvar);

    // assign RTO (here: rexmit_timeout) a new value
    double rto = srtt + 4*rttvar;
    if (rto>MAX_REXMIT_TIMEOUT)
        rto = MAX_REXMIT_TIMEOUT;
    else if (rto<MIN_REXMIT_TIMEOUT)
        rto = MIN_REXMIT_TIMEOUT;

    ASSERT(!rexmitTimer->isScheduled());  // RTT measurement is only successful if there's no retransmission
    state->rexmit_timeout = rto;
}

void PlainTCP::sendData()
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

    //
    // Slow start, congestion control etc.: send window is effectively the
    // minimum of the congestion window (cwnd) and the advertised window (snd_wnd).
    //
    int window = Min(state->snd_cwnd, state->snd_wnd);

    // OK, do it
    conn->sendData(fullSegmentsOnly, window/state->snd_mss); // FIXME should take bytes rather than numsegments...
}

void PlainTCP::sendCommandInvoked()
{
    sendData();
}

void PlainTCP::receiveSeqChanged()
{
    if (!state->delayed_acks_enabled)
    {
        tcpEV << "Sending ACK now (delayed ACKs are disabled)\n";
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

void PlainTCP::receivedDataAck()
{
    state->dupacks = 0;

    // handling of retransmission timer: (as in SSFNet):
    // "if the ACK is for the last segment sent (no data in flight), cancel
    // the timer, else restart the timer with the current RTO value."
    if (state->snd_una==state->snd_max)
    {
        tcpEV << "All outstanding segments acked, cancelling REXMIT timer (if running)\n";
        conn->getTcpMain()->cancelEvent(rexmitTimer);
    }
    else
    {
        tcpEV << "Some but not all outstanding segments acked, restarting REXMIT timer\n";
        startRexmitTimer();
    }

    // if round-trip time measurement is running, check if rtseq has been acked
    if (state->t_rtseq_sent!=0 && seqLess(state->rtseq, state->snd_una))
    {
        // print value
        tcpEV << "Round-trip time measured on rtseq=" << state->rtseq << ": "
              << int((conn->getTcpMain()->simTime() - state->t_rtseq_sent)*1000+0.5) << "ms\n";

        // update RTT variables with new value
        rttMeasurementComplete(state->t_rtseq_sent, conn->getTcpMain()->simTime());

        // measurement finished
        state->t_rtseq_sent = 0;
    }

    // slow start, congestion avoidance etc.:
    if (state->snd_cwnd <= state->ssthresh)
    {
        // perform Slow Start
        state->snd_cwnd += state->snd_mss;
    }
    else
    {
        // perform Congestion Avoidance
        state->snd_cwnd += state->snd_mss * state->snd_mss / state->snd_cwnd;
    }

    // ack may have freed up some room in the window, try sending
    sendData();
}

void PlainTCP::receivedDuplicateAck()
{
    state->dupacks++;    // FIXME move this into TCPConn? if doesn't qualify as dupack, still dupack=0 must be done!

    // perform Fast Retransmission (Tahoe) or Fast Retransmission/Recovery (Reno)
    if (state->dupacks>=3)
    {
        // FIXME todo
    }
}

void PlainTCP::receivedAckForDataNotYetSent(uint32 seq)
{
    state->dupacks = 0;
    tcpEV << "Sending immediate ACK\n";
    conn->sendAck();
}

void PlainTCP::ackSent()
{
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        conn->getTcpMain()->cancelEvent(delayedAckTimer);
}

void PlainTCP::dataSent(uint32 fromseq)
{
    // if retransmission timer not running, schedule it
    if (!rexmitTimer->isScheduled())
    {
        tcpEV << "Starting REXMIT timer\n";
        startRexmitTimer();
    }

    // start round-trip time measurement (if not already running)
    if (state->t_rtseq_sent==0)
    {
        // remember this sequence number and when it was sent
        state->rtseq = fromseq;
        state->t_rtseq_sent = conn->getTcpMain()->simTime();
    }

}

void PlainTCP::dataRetransmitted()
{
}


