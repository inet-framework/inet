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

#ifndef __PLAINTCP_H
#define __PLAINTCP_H

#include <omnetpp.h>
#include "TCPAlgorithm.h"


/**
 * State variables for PlainTCP.
 */
class PlainTCPStateVariables : public TCPStateVariables
{
  public:
    PlainTCPStateVariables();

    // retransmit count
    uint32 rexmit_seq;      // sequence number retransmitted
    int rexmit_count;       // number of retransmissions (=1 after first rexmit)
    int rexmit_timeout;     // current retransmission timeout

    // slow start and congestion avoidance variables (RFC 2001)
    //int snd_cwnd;          // congestion window
    //int ssthresh;          // slow start threshold

    // receive variables
    //uint32 rcv_fin_seq;
    //bool rcv_fin_valid;
    //bool rcv_up_valid;
    //uint32 rcv_buf_seq;
    //unsigned long rcv_buff;
    //double  rcv_buf_usage_thresh;

    // retransmit variables
    //uint32 snd_max;      // highest sequence number sent; used to recognize retransmits
    //uint32 max_retrans_seq; // sequence number of a retransmitted segment

    // timing information (round-trip)
    //short t_rtt;                // round-trip time
    //uint32 rtseq;             // starting sequence number of timed data
    //short rttmin;
    //short srtt;                 // smoothed round-trip time
    //short rttvar;               // variance of round-trip time
    //double last_timed_data;     // timestamp for measurement

    // retransmission timeout
    //short rxtcur;
    // backoff for rto
    //short rxtshift;

    // duplicate ack counter
    short dupacks;

    // last time a segment was send
    //double last_snd_time;

    // ACK times
    //double ack_send_time;
    //double ack_rcv_time;
};


/**
 * Includes basic TCP algorithms: retransmission, PERSIST timer, keep-alive,
 * delayed acknowledge.
 */
class PlainTCP : public TCPAlgorithm
{
  protected:
    cMessage *rexmitTimer;
    cMessage *persistTimer;
    cMessage *delayedAckTimer;
    cMessage *keepAliveTimer;

    PlainTCPStateVariables *state;

  protected:
    /** @name Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers */
    //@{
    virtual void processRexmitTimer(TCPEventCode& event);
    virtual void processPersistTimer(TCPEventCode& event);
    virtual void processDelayedAckTimer(TCPEventCode& event);
    virtual void processKeepAliveTimer(TCPEventCode& event);
    //@}
  public:
    /**
     * Ctor.
     */
    PlainTCP();

    /**
     * Virtual dtor.
     */
    virtual ~PlainTCP();

    /**
     * Create timers, etc.
     */
    virtual void initialize();

    /**
     * Create and return a PlainTCPStateVariables object.
     */
    virtual TCPStateVariables *createStateVariables();

    /**
     * Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers.
     */
    virtual void processTimer(cMessage *timer, TCPEventCode& event);

    virtual void sendCommandInvoked();

    virtual void receiveSeqChanged();

    virtual void receivedAck(bool duplicate);

    virtual void receivedAckForDataNotYetSent(uint32 seq);

    virtual void ackSent();

    virtual void dataSent();

    virtual void dataRetransmitted();

};

#endif


