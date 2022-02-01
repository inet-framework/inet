//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/DumbTcp.h"

#include "inet/transportlayer/tcp/Tcp.h"

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

        if (!conn->sendData(65535))
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
    conn->scheduleAfter(REXMIT_TIMEOUT, rexmitTimer);
}

void DumbTcp::sendCommandInvoked()
{
    // start sending
    conn->sendData(65535);
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

void DumbTcp::receivedDataAck(uint32_t)
{
    // ack may have freed up some room in the window, try sending.
    conn->sendData(65535);
}

void DumbTcp::receivedDuplicateAck()
{
    EV_INFO << "Duplicate ACK #" << state->dupacks << "\n";
}

void DumbTcp::receivedAckForDataNotYetSent(uint32_t seq)
{
    EV_INFO << "ACK acks something not yet sent, sending immediate ACK\n";
    conn->sendAck();
}

void DumbTcp::ackSent()
{
}

void DumbTcp::dataSent(uint32_t fromseq)
{
    conn->rescheduleAfter(REXMIT_TIMEOUT, rexmitTimer);
}

void DumbTcp::segmentRetransmitted(uint32_t fromseq, uint32_t toseq)
{
}

void DumbTcp::restartRexmitTimer()
{
}

void DumbTcp::rttMeasurementCompleteUsingTS(uint32_t echoedTS)
{
}

bool DumbTcp::shouldMarkAck()
{
    return false;
}

void DumbTcp::processEcnInEstablished()
{
}

} // namespace tcp
} // namespace inet

