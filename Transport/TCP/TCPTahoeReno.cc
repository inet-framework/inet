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

#include "TCPTahoeReno.h"
#include "TCPMain.h"


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

Register_Class(TCPTahoeReno);


TCPTahoeRenoStateVariables::TCPTahoeRenoStateVariables()
{
    delayed_acks_enabled = true;
    nagle_enabled = true;
    tcpvariant = TAHOE;

    rexmit_count = 0;
    rexmit_timeout = 3.0;

    snd_cwnd = 0; // will be set to MSS when connection is established
    ssthresh = 65535;

    rtseq = 0;
    t_rtseq_sent = 0;

    // Jacobson's alg: srtt must be initialized to 0, rttvar to a value which
    // will yield rto = 3s initially.
    srtt = 0;
    rttvar = 3.0/4.0;
}


TCPTahoeReno::TCPTahoeReno() : TCPAlgorithm()
{
    rexmitTimer = persistTimer = delayedAckTimer = keepAliveTimer = NULL;
    state = NULL;
}

TCPTahoeReno::~TCPTahoeReno()
{
    // Note: don't delete "state" here, it'll be deleted from TCPConnection

    // cancel and delete timers
    if (rexmitTimer)     delete cancelEvent(rexmitTimer);
    if (persistTimer)    delete cancelEvent(persistTimer);
    if (delayedAckTimer) delete cancelEvent(delayedAckTimer);
    if (keepAliveTimer)  delete cancelEvent(keepAliveTimer);
}

void TCPTahoeReno::initialize()
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

TCPStateVariables *TCPTahoeReno::createStateVariables()
{
    ASSERT(state==NULL);
    state = new TCPTahoeRenoStateVariables();
    return state;
}

void TCPTahoeReno::established(bool active)
{
    // initialize cwnd (we may learn MSS during connection setup --
    // this (MSS TCP option) is not implemented yet though)
    state->snd_cwnd = state->snd_mss;

    if (active)
    {
        // finish connection setup with ACK (possibly piggybacked on data)
        tcpEV << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData())
            conn->sendAck();
    }
}

void TCPTahoeReno::connectionClosed()
{
    cancelEvent(rexmitTimer);
    cancelEvent(persistTimer);
    cancelEvent(delayedAckTimer);
    cancelEvent(keepAliveTimer);
}

void TCPTahoeReno::processTimer(cMessage *timer, TCPEventCode& event)
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

void TCPTahoeReno::processRexmitTimer(TCPEventCode& event)
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
        tcpEV << "Retransmission count exceeds " << MAX_REXMIT_COUNT << ", aborting connection\n";
        conn->signalConnectionTimeout();
        event = TCP_E_ABORT;  // TBD maybe rather introduce a TCP_E_TIMEDOUT event
        return;
    }

    tcpEV << "Performing retransmission #" << state->rexmit_count << "\n";

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
    // Slow Start, Congestion Control (RFC2001)
    //
    uint flight_size = Min(state->snd_cwnd, state->snd_wnd);
    state->ssthresh = Max(flight_size/2, 2*state->snd_mss);
    state->snd_cwnd = state->snd_mss;

    // retransmit
    conn->retransmitData();
}

void TCPTahoeReno::processPersistTimer(TCPEventCode& event)
{
    // FIXME TBD
}

void TCPTahoeReno::processDelayedAckTimer(TCPEventCode& event)
{
    conn->sendAck();
}

void TCPTahoeReno::processKeepAliveTimer(TCPEventCode& event)
{
    // FIXME TBD
}

void TCPTahoeReno::startRexmitTimer()
{
    // start counting retransmissions for this seq number.
    // Note: state->rexmit_timeout is set from rttMeasurementComplete().
    state->rexmit_count = 0;

    // schedule timer
    conn->scheduleTimeout(rexmitTimer, state->rexmit_timeout);
}

void TCPTahoeReno::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
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

bool TCPTahoeReno::sendData()
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
        tcpEV << "Nagle is enabled and there's unack'ed data: only full segments will be sent\n";

    //
    // Slow start, congestion control etc.: send window is effectively the
    // minimum of the congestion window (cwnd) and the advertised window (snd_wnd).
    //
    return conn->sendData(fullSegmentsOnly, state->snd_cwnd);
}

void TCPTahoeReno::sendCommandInvoked()
{
    // FIXME there's a problem with this: it interferes with slow start.
    // Every time we get a SEND command we send up to cwnd bytes, whether
    // we received an ack in between or not... See nagle_2.test.

    // TBD perhaps this should be invoked only if sendQueue was previously empty?
    sendData();
}

void TCPTahoeReno::receivedOutOfOrderSegment()
{
    tcpEV << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void TCPTahoeReno::receiveSeqChanged()
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

void TCPTahoeReno::receivedDataAck(uint32 firstSeqAcked)
{
    // first cancel retransmission timer
    cancelEvent(rexmitTimer);

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

    // handling of retransmission timer: (as in SSFNet):
    // "if the ACK is for the last segment sent (no data in flight), cancel
    // the timer, else restart the timer with the current RTO value."
    if (state->snd_una==state->snd_max)
    {
        tcpEV << "ACK acks all outstanding segments\n";
    }
    else
    {
        tcpEV << "ACK acks some but not all outstanding segments, restarting REXMIT timer\n";
        startRexmitTimer();
    }

    if (state->tcpvariant==TCPTahoeRenoStateVariables::RENO && state->dupacks>=3)
    {
        //
        // Reno: if we're just after a Fast Retransmission, perform Fast Recovery:
        // set cwnd to ssthresh (deflating the window).
        //
        state->snd_cwnd = state->ssthresh;
    }
    else
    {
        //
        // Perform slow start and congestion avoidance. RFC2001: "Slow start has
        // cwnd begin at one segment, and be incremented by one segment every time
        // an ACK is received."  NOTE: "every time" means "for every segment
        // acknowledged!"
        //
        int bytesAcked = state->snd_una - firstSeqAcked;
        int segmentsAcked = Max(1,bytesAcked/state->snd_mss); // probably shouldn't be zero. FIXME is this right?

        if (state->snd_cwnd <= state->ssthresh)
        {
            // perform Slow Start
            state->snd_cwnd += segmentsAcked * state->snd_mss;
        }
        else
        {
            // perform Congestion Avoidance
            for (int i=0; i<segmentsAcked; i++)
                state->snd_cwnd += state->snd_mss * state->snd_mss / state->snd_cwnd;
        }
    }

    // ack may have freed up some room in the window, try sending
    sendData();
}

void TCPTahoeReno::receivedDuplicateAck()
{
    tcpEV << "Duplicate ACK #" << state->dupacks << "\n";

    if (state->dupacks==3)
    {
        //
        // perform Fast Retransmission (Tahoe) or Fast Retransmission/Recovery (Reno)
        //

        // Reset ssthresh and cwnd. Note: code is similar to that in processRexmitTimer()
        uint flight_size = Min(state->snd_cwnd, state->snd_wnd);
        state->ssthresh = Max(flight_size/2, 2*state->snd_mss);
        if (state->tcpvariant==TCPTahoeRenoStateVariables::TAHOE)
            state->snd_cwnd = state->snd_mss;
        else if (state->tcpvariant==TCPTahoeRenoStateVariables::RENO)
            state->snd_cwnd = 3*state->snd_mss;
        else
            ASSERT(0);

        tcpEV << "Performing Fast Retransmit, new cwnd=" << state->snd_cwnd << "\n";

        // cancel round-trip time measurement
        state->t_rtseq_sent = 0;

        // restart retransmission timer (with rexmit_count=0). RTO is unchanged.
        cancelEvent(rexmitTimer);
        startRexmitTimer();

        // retransmit missing segment
        conn->retransmitData();
    }
    else if (state->dupacks > 3)
    {
        //
        // Reno: For each additional duplicate ACK received, increment cwnd by MSS.
        // This artificially inflates the congestion window in order to reflect the
        // additional segment that has left the network
        //
        if (state->tcpvariant==TCPTahoeRenoStateVariables::RENO)
        {
            state->snd_cwnd += state->snd_mss;
            tcpEV << "Reno: inflating cwnd by MSS, new cwnd=" << state->snd_cwnd << "\n";
        }
        // FIXME shouldn't we try to transmit, retransmit or something like that here?
    }
}

void TCPTahoeReno::receivedAckForDataNotYetSent(uint32 seq)
{
    tcpEV << "ACK acks something not yet sent, sending immediate ACK\n";
    conn->sendAck();
}

void TCPTahoeReno::ackSent()
{
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        cancelEvent(delayedAckTimer);
}

void TCPTahoeReno::dataSent(uint32 fromseq)
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



