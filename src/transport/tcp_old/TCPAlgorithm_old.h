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

#ifndef __INET_TCPALGORITHM_OLD_H
#define __INET_TCPALGORITHM_OLD_H

#include <omnetpp.h>
#include "TCPConnection_old.h"
#include "TCPSegment.h"

namespace tcp_old {

/**
 * Abstract base class for TCP algorithms which encapsulate all behaviour
 * during data transfer state: flavour of congestion control, fast
 * retransmit/recovery, selective acknowledgement etc. Subclasses
 * may implement various sets and flavours of the above algorithms.
 */
class INET_API TCPAlgorithm : public cPolymorphic
{
  protected:
    TCPConnection *conn; // we belong to this connection
    TCPStateVariables *state; // our state variables

    /**
     * Create state block (TCB) used by this TCP variant. It is expected
     * that every TCPAlgorithm subclass will have its own state block,
     * subclassed from TCPStateVariables. This factory method should
     * create and return a "blank" state block of the appropriate type.
     */
    virtual TCPStateVariables *createStateVariables() = 0;

  public:
    /**
     * Ctor.
     */
    TCPAlgorithm() {state = NULL; conn = NULL;}

    /**
     * Virtual dtor.
     */
    virtual ~TCPAlgorithm() {}

    /**
     * Assign this object to a TCPConnection. Its sendQueue and receiveQueue
     * must be set already at this time, because we cache their pointers here.
     */
    void setConnection(TCPConnection *_conn)  {conn = _conn;}

    /**
     * Creates and returns the TCP state variables.
     */
    TCPStateVariables *getStateVariables() {
        if (!state) state = createStateVariables();
        return state;
    }

    /**
     * Should be redefined to initialize the object: create timers, etc.
     * This method is necessary because the TCPConnection ptr is not
     * available in the constructor yet.
     */
    virtual void initialize() {}

    /**
     * Called when the connection is going to ESTABLISHED from SYN_SENT or
     * SYN_RCVD. This is a place to initialize some variables (e.g. set
     * cwnd to the MSS learned during connection setup). If we are on the
     * active side, here we also have to finish the 3-way connection setup
     * procedure by sending an ACK, possibly piggybacked on data.
     */
    virtual void established(bool active) = 0;

    /**
     * Called when the connection closes, it should cancel all running timers.
     */
    virtual void connectionClosed() = 0;

    /**
     * Place to process timers specific to this TCPAlgorithm class.
     * TCPConnection will invoke this method on any timer (self-message)
     * it doesn't recognize (that is, any timer other than the 2MSL,
     * CONN-ESTAB and FIN-WAIT-2 timers).
     *
     * Method may also change the event code (by default set to TCP_E_IGNORE)
     * to cause the state transition of TCP FSM.
     */
    virtual void processTimer(cMessage *timer, TCPEventCode& event) = 0;

    /**
     * Called after user sent TCP_C_SEND command to us.
     */
    virtual void sendCommandInvoked() = 0;

    /**
     * Called after receiving data which are in the window, but not at its
     * left edge (seq!=rcv_nxt). This indicates that either segments got
     * re-ordered in the way, or one segment was lost. RFC1122 and RFC2001
     * recommend sending an immediate ACK here (Fast Retransmit relies on
     * that).
     */
    virtual void receivedOutOfOrderSegment() = 0;

    /**
     * Called after rcv_nxt got advanced, either because we received in-sequence
     * data ("text" in RFC 793 lingo) or a FIN. At this point, rcv_nxt has
     * already been updated. This method should take care to send or schedule
     * an ACK some time.
     */
    virtual void receiveSeqChanged() = 0;

    /**
     * Called after we received an ACK which acked some data (that is,
     * we could advance snd_una). At this point the state variables
     * (snd_una, snd_wnd) have already been updated. The argument firstSeqAcked
     * is the previous snd_una value, that is, the number of bytes acked is
     * (snd_una-firstSeqAcked). The dupack counter still reflects the old value
     * (needed for Reno and NewReno); it'll be reset to 0 after this call returns.
     */
    virtual void receivedDataAck(uint32 firstSeqAcked) = 0;

    /**
     * Called after we received a duplicate ACK (that is: ackNo==snd_una,
     * no data in segment, segment doesn't carry window update, and also,
     * we have unacked data). The dupack counter got already updated
     * when calling this method (i.e. dupack==1 on first duplicate ACK.)
     */
    virtual void receivedDuplicateAck() = 0;

    /**
     * Called after we received an ACK for data not yet sent.
     * According to RFC 793 this function should send an ACK.
     */
    virtual void receivedAckForDataNotYetSent(uint32 seq) = 0;

    /**
     * Called after we sent an ACK. This hook can be used to cancel
     * the delayed-ACK timer.
     */
    virtual void ackSent() = 0;

    /**
     * Called after we sent data. This hook can be used to schedule the
     * retransmission timer, to start round-trip time measurement, etc.
     * The argument is the seqno of the first byte sent.
     */
    virtual void dataSent(uint32 fromseq) = 0;

    /**
     * Restart REXMIT timer.
     */
    virtual void restartRexmitTimer() = 0;

};

}
#endif


