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

#ifndef __TCPTAHOERENO_H
#define __TCPTAHOERENO_H

#include <omnetpp.h>
#include "TCPAlgorithm.h"


/**
 * State variables for TCPTahoeReno.
 */
class TCPTahoeRenoStateVariables : public TCPStateVariables
{
  public:
    TCPTahoeRenoStateVariables();

    // TCP features
    bool delayed_acks_enabled; // delayed ACKs enabled/disabled; FIXME make this a socket option
    bool nagle_enabled;        // Nagle's algorithm (off = NODELAY socket option)
    enum {TAHOE,RENO} tcpvariant;  // TCP algorithm

    // retransmit count
    int rexmit_count;         // number of retransmissions (=1 after first rexmit)
    simtime_t rexmit_timeout; // current retransmission timeout (aka RTO)

    // slow start and congestion avoidance variables (RFC 2001)
    uint snd_cwnd;            // congestion window
    uint ssthresh;            // slow start threshold

    // performing round-trip time measurements
    uint32 rtseq;            // starting sequence number of timed data
    simtime_t t_rtseq_sent;  // time when rtseq was sent (0 if RTT measurement is not running)

    // round-trip time estimation (Jacobson's algorithm)
    simtime_t srtt;          // smoothed round-trip time
    simtime_t rttvar;        // variance of round-trip time

    // last time a segment was sent
    //double last_snd_time;

    // ACK times
    //double ack_send_time;
    //double ack_rcv_time;
};


/**
 * Includes basic TCP algorithms: adaptive retransmission, persist timer,
 * keep-alive, delayed acks, congestion control.
 *
 * Implements:
 *   - delayed acks
 *   - Jacobson's and Karn's algorithms for adaptive retransmission
 *   - Nagle's algorithm to prevent silly window syndrome
 *   - Tahoe (Fast Rexmit) or Reno (Fast Rexmit/Fast Recovery)
 * To be done:
 *   - persist timer, keepalive timer
 *
 * Note: currently the timers and time calculations are done in double
 * and NOT in Unix (200ms or 500ms) ticks. It's possible to write another
 * TCPAlgorithm which uses ticks (or rather, factor out timer handling to
 * separate methods, and redefine only those).
 */
class TCPTahoeReno : public TCPAlgorithm
{
  protected:
    cMessage *rexmitTimer;
    cMessage *persistTimer;
    cMessage *delayedAckTimer;
    cMessage *keepAliveTimer;

    TCPTahoeRenoStateVariables *state;

  protected:
    /** @name Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers */
    //@{
    virtual void processRexmitTimer(TCPEventCode& event);
    virtual void processPersistTimer(TCPEventCode& event);
    virtual void processDelayedAckTimer(TCPEventCode& event);
    virtual void processKeepAliveTimer(TCPEventCode& event);
    //@}

    /**
     * Start REXMIT timer and initialize retransmission variables
     */
    virtual void startRexmitTimer();

    /**
     * Update state vars with new measured RTT value. Passing two simtime_t's
     * will allow rttMeasurementComplete() to do calculations in double or
     * in 200ms/500ms ticks, as needed)
     */
    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked);

    /**
     * Send data, observing Nagle's algorithm and congestion window
     */
    virtual bool sendData();

    /** Utility function */
    cMessage *cancelEvent(cMessage *msg)  {return conn->getTcpMain()->cancelEvent(msg);}

  public:
    /**
     * Ctor.
     */
    TCPTahoeReno();

    /**
     * Virtual dtor.
     */
    virtual ~TCPTahoeReno();

    /**
     * Create timers, etc.
     */
    virtual void initialize();

    /**
     * Create and return a TCPTahoeRenoStateVariables object.
     */
    virtual TCPStateVariables *createStateVariables();

    virtual void established(bool active);

    virtual void connectionClosed();

    /**
     * Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers.
     */
    virtual void processTimer(cMessage *timer, TCPEventCode& event);

    virtual void sendCommandInvoked();

    virtual void receivedOutOfOrderSegment();

    virtual void receiveSeqChanged();

    virtual void receivedDataAck(uint32 firstSeqAcked);

    virtual void receivedDuplicateAck();

    virtual void receivedAckForDataNotYetSent(uint32 seq);

    virtual void ackSent();

    virtual void dataSent(uint32 fromseq);

};

#endif


