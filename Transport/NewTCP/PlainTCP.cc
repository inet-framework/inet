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
#define MAX_REXMIT_TIMEOUT    128   // 2min

Register_Class(PlainTCP);


PlainTCPStateVariables::PlainTCPStateVariables()
{
    rexmit_seq = 0;
    rexmit_count = 0;
    rexmit_timeout = 0.0;
    dupacks = 0;
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
    if (state->snd_una!=state->rexmit_seq)
    {
        // start counting retransmissions for this seq number
        state->rexmit_seq = state->snd_una;
        state->rexmit_count = 1;  // FIXME move it to where rexmit timer is started in 1st place!!!!!!!
        state->rexmit_timeout = 3.0;  // FIXME use dynamically calculated RTO variable
    }
    else
    {
        // if reaches max count, abort connection
        if (++state->rexmit_count > MAX_REXMIT_COUNT)
        {
            event = TCP_E_ABORT;  // TBD maybe rather introduce a TCP_E_BROKEN event
            return;
        }
    }

    // restart the retransmission timer with twice the latest RTO value, or with the max, whichever is smaller
    state->rexmit_timeout += state->rexmit_timeout;
    if (state->rexmit_timeout > MAX_REXMIT_TIMEOUT)
        state->rexmit_timeout = MAX_REXMIT_TIMEOUT;

    // retransmit & restart timer
    conn->retransmitData();
    conn->scheduleTimeout(rexmitTimer, state->rexmit_timeout);
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

void PlainTCP::sendCommandInvoked()
{
    conn->sendData();
}

void PlainTCP::receiveSeqChanged()
{
    // FIXME ACK should be generated for at least every second SMSS-sized segment!
    // schedule delayed ACK timer if not already running
    tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", scheduling ACK\n";
    if (!delayedAckTimer->isScheduled())
        conn->scheduleTimeout(delayedAckTimer, DELAYED_ACK_TIMEOUT);
}

void PlainTCP::receivedAck(bool duplicate)
{
    if (duplicate)
    {
        // take care of duplicate ACKs then return
        state->dupacks++;
        return;
    }
    state->dupacks = 0;

    // handling of retransmission timer: (as in SSFNet):
    // "if the ACK is for the last segment sent (no data in flight), cancel
    // the timer, else restart the timer with the current RTO value."
    if (state->snd_una==state->snd_nxt)  // FIXME maybe snd_max instead of snd_nxt? (snd_max not yet maintained!)
    {
        tcpEV << "All outstanding segments acked, cancelling REXMIT timer (if running)\n";
        conn->getTcpMain()->cancelEvent(rexmitTimer);
    }
    else
    {
        tcpEV << "Some but not all outstanding segments acked, rescheduling REXMIT timer\n";
        conn->scheduleTimeout(rexmitTimer, 1.0); // FIXME use dynamically calculated RTO variable
    }

    // ack may have freed up some room in the window, try sending
    conn->sendData();
}

void PlainTCP::receivedAckForDataNotYetSent(uint32 seq)
{
    // FIXME might be incorrect
    tcpEV << "Sending immediate ACK\n";
    conn->sendAck();
}

void PlainTCP::ackSent()
{
    // if delayed ACK timer is running, cancel it
    if (delayedAckTimer->isScheduled())
        conn->getTcpMain()->cancelEvent(delayedAckTimer);
}

void PlainTCP::dataSent()
{
    // if retransmission timer not running, schedule it
    if (!rexmitTimer->isScheduled())
    {
        tcpEV << "Scheduling REXMIT timer\n";
        conn->scheduleTimeout(rexmitTimer, 1.0); // FIXME use dynamically calculated RTO variable
    }
}

void PlainTCP::dataRetransmitted()
{
}


