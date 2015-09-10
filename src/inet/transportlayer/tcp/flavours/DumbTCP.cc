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

#include "inet/transportlayer/tcp/flavours/DumbTCP.h"
#include "inet/transportlayer/tcp/TCP.h"

namespace inet {

namespace tcp {

Register_Class(DumbTCP);

// just a dummy value
#define REXMIT_TIMEOUT    2

DumbTCP::DumbTCP() : TCPAlgorithm(),
    state((DumbTCPStateVariables *&)TCPAlgorithm::state)
{
    rexmitTimer = nullptr;
}

DumbTCP::~DumbTCP()
{
    // cancel and delete timers
    if (rexmitTimer)
        delete conn->getTcpMain()->cancelEvent(rexmitTimer);
}

void DumbTCP::initialize()
{
    TCPAlgorithm::initialize();

    rexmitTimer = new cMessage("REXMIT");
    rexmitTimer->setContextPointer(conn);
}

void DumbTCP::established(bool active)
{
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";

        if (!conn->sendData(false, 65535))
            conn->sendAck();
    }
}

void DumbTCP::connectionClosed()
{
    conn->getTcpMain()->cancelEvent(rexmitTimer);
}

void DumbTCP::processTimer(cMessage *timer, TCPEventCode& event)
{
    if (timer != rexmitTimer)
        throw cRuntimeError(timer, "unrecognized timer");

    conn->retransmitData();
    conn->scheduleTimeout(rexmitTimer, REXMIT_TIMEOUT);
}

void DumbTCP::sendCommandInvoked()
{
    // start sending
    conn->sendData(false, 65535);
}

void DumbTCP::receivedOutOfOrderSegment()
{
    EV_INFO << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void DumbTCP::receiveSeqChanged()
{
    // new data received, ACK immediately (more sophisticated algs should
    // wait a little to see if piggybacking is possible)
    EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", sending immediate ACK\n";
    conn->sendAck();
}

void DumbTCP::receivedDataAck(uint32)
{
    // ack may have freed up some room in the window, try sending.
    // small segments also OK (Nagle off)
    conn->sendData(false, 65535);
}

void DumbTCP::receivedDuplicateAck()
{
    EV_INFO << "Duplicate ACK #" << state->dupacks << "\n";
}

void DumbTCP::receivedAckForDataNotYetSent(uint32 seq)
{
    EV_INFO << "ACK acks something not yet sent, sending immediate ACK\n";
    conn->sendAck();
}

void DumbTCP::ackSent()
{
}

void DumbTCP::dataSent(uint32 fromseq)
{
    if (rexmitTimer->isScheduled())
        conn->getTcpMain()->cancelEvent(rexmitTimer);

    conn->scheduleTimeout(rexmitTimer, REXMIT_TIMEOUT);
}

void DumbTCP::segmentRetransmitted(uint32 fromseq, uint32 toseq)
{
}

void DumbTCP::restartRexmitTimer()
{
}

void DumbTCP::rttMeasurementCompleteUsingTS(uint32 echoedTS)
{
}

} // namespace tcp

} // namespace inet

