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

#include "DummyTCPAlg.h"
#include "TCPMain.h"

Register_Class(DummyTCPAlg);


DummyTCPAlg::DummyTCPAlg() : TCPAlgorithm()
{
    state = NULL;
}

DummyTCPAlg::~DummyTCPAlg()
{
    // Note: don't delete "state" here, it'll be deleted from TCPConnection
}

TCPStateVariables *DummyTCPAlg::createStateVariables()
{
    ASSERT(state==NULL);
    state = new DummyTCPStateVariables();
    return state;
}

void DummyTCPAlg::established()
{
}

void DummyTCPAlg::processTimer(cMessage *timer, TCPEventCode& event)
{
    // no extra timers in this TCP variant
}

void DummyTCPAlg::sendCommandInvoked()
{
    // start sending as much as possible, small segments also OK (Nagle off)
    conn->sendData(false);
}

void DummyTCPAlg::receivedOutOfOrderSegment()
{
    tcpEV << "out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void DummyTCPAlg::receiveSeqChanged()
{
    // new data received, ACK immediately (more sophisticated algs should
    // wait a little to see if piggybacking is possible)
    tcpEV << "rcv_nxt changed to " << state->rcv_nxt << ", sending immediate ACK\n";
    conn->sendAck();
}

void DummyTCPAlg::receivedDataAck(uint32)
{
    // ack may have freed up some room in the window, try sending.
    // small segments also OK (Nagle off)
    conn->sendData(false);
}

void DummyTCPAlg::receivedDuplicateAck()
{
}

void DummyTCPAlg::receivedAckForDataNotYetSent(uint32 seq)
{
    // more sophisticated algs whould interpret this specially, but we
    // just send a "correct" ack
    tcpEV << "Sending immediate ACK\n";
    conn->sendAck();
}

void DummyTCPAlg::ackSent()
{
}

void DummyTCPAlg::dataSent(uint32)
{
}

void DummyTCPAlg::dataRetransmitted()
{
}

