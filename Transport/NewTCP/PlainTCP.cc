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

Register_Class(PlainTCP);

PlainTCP::PlainTCP() : TCPAlgorithm()
{
    rexmitTimer = NULL;
    persistTimer = NULL;
    delayedAckTimer = NULL;
    keepAliveTimer = NULL;
}

PlainTCP::~PlainTCP()
{
    // FIXME cancel & delete timers
}

TCPStateVariables *PlainTCP::createStateVariables()
{
    return new PlainTCPStateVariables();
}

void PlainTCP::processTimer(cMessage *timer, TCPEventCode& event)
{
    // FIXME Process REXMIT, PERSIST, DELAYED-ACK and KEEP-ALIVE timers.
}

/*
void PlainTCP::process_TIMEOUT_REXMT(TCPEventCode& event, cMessage *msg)
{
    //"
    // For any state if the retransmission timeout expires on a segment in
    // the retransmission queue, send the segment at the front of the
    // retransmission queue again, reinitialize the retransmission timer,
    // and return.
    //"

    // FIXME TBD
}
*/

void PlainTCP::sendCommandInvoked()
{
    conn->sendData();
}

void PlainTCP::receivedSegmentText()
{
    //FIXME: dummy stuff only for testing:
    // immediate ACK:
    conn->sendAck();
}

void PlainTCP::receivedAck()
{
    //FIXME dummy code only for testing!!
    conn->sendData();
}

void PlainTCP::receivedAckForDataNotYetSent(uint32 seq)
{
    // immediate ACK: (FIXME for testing only)
    ev << "Sending immediate ACK\n";
    conn->sendAck();
}

