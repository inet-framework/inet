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

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/flavours/DumbTcp.h"

namespace inet {
namespace tcp {

Register_Class(DumbTcp);

// just a dummy value
#define REXMIT_TIMEOUT    2

DumbTcp::DumbTcp() : TcpAlgorithm(),
    state(reinterpret_cast<DumbTcpStateVariables *&>(TcpAlgorithm::state))
{
    rexmitTimer = nullptr;
}

DumbTcp::~DumbTcp()
{
    // cancel and delete timers
    if (rexmitTimer)
        delete conn->cancelEvent(rexmitTimer);
}

void DumbTcp::initialize()
{
    TcpAlgorithm::initialize();

    rexmitTimer = new cMessage("REXMIT");
    rexmitTimer->setContextPointer(conn);
}

void DumbTcp::established(bool active)
{
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";

        if (!conn->sendData(false, 65535))
            conn->sendAck();
    }
}

void DumbTcp::connectionClosed()
{
    conn->cancelEvent(rexmitTimer);
}

void DumbTcp::processTimer(cMessage *timer, TcpEventCode& event)
{
    if (timer != rexmitTimer)
        throw cRuntimeError(timer, "unrecognized timer");

    conn->retransmitData();
    conn->scheduleTimeout(rexmitTimer, REXMIT_TIMEOUT);
}

void DumbTcp::sendCommandInvoked()
{
    // start sending
    conn->sendData(false, 65535);
}

void DumbTcp::receivedOutOfOrderSegment()
{
    EV_INFO << "Out-of-order segment, sending immediate ACK\n";
    conn->sendAck();
}

void DumbTcp::receiveSeqChanged()
{
    // new data received, ACK immediately (more sophisticated algs should
    // wait a little to see if piggybacking is possible)
    EV_INFO << "rcv_nxt changed to " << state->rcv_nxt << ", sending immediate ACK\n";
    conn->sendAck();
}

void DumbTcp::receivedDataAck(uint32)
{
    // ack may have freed up some room in the window, try sending.
    // small segments also OK (Nagle off)
    conn->sendData(false, 65535);
}

void DumbTcp::receivedDuplicateAck()
{
    EV_INFO << "Duplicate ACK #" << state->dupacks << "\n";
}

void DumbTcp::receivedAckForDataNotYetSent(uint32 seq)
{
    EV_INFO << "ACK acks something not yet sent, sending immediate ACK\n";
    conn->sendAck();
}

void DumbTcp::ackSent()
{
}

void DumbTcp::dataSent(uint32 fromseq)
{
    if (rexmitTimer->isScheduled())
        conn->cancelEvent(rexmitTimer);

    conn->scheduleTimeout(rexmitTimer, REXMIT_TIMEOUT);
}

void DumbTcp::segmentRetransmitted(uint32 fromseq, uint32 toseq)
{
}

void DumbTcp::restartRexmitTimer()
{
}

void DumbTcp::rttMeasurementCompleteUsingTS(uint32 echoedTS)
{
}

} // namespace tcp

} // namespace inet

