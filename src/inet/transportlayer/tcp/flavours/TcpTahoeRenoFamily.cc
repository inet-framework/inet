//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

void TcpTahoeRenoFamilyStateVariables::setSendQueueLimit(uint32_t newLimit) {
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window) -> defined by sendQueueLimit
    sendQueueLimit = newLimit;
    ssthresh = sendQueueLimit;
}

std::string TcpTahoeRenoFamilyStateVariables::str() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpTahoeRenoFamilyStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

// ---

TcpTahoeRenoFamily::TcpTahoeRenoFamily() : TcpAlgorithmBase(),
    state((TcpTahoeRenoFamilyStateVariables *&)TcpAlgorithm::state)
{
}

TcpTahoeRenoFamily::~TcpTahoeRenoFamily()
{
    delete congestionControl;
    delete recovery;
}

void TcpTahoeRenoFamily::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

void TcpTahoeRenoFamily::established(bool active)
{
    TcpAlgorithmBase::established(active);

    recovery = createRecovery();
    congestionControl = createCongestionControl();
}

void TcpTahoeRenoFamily::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // RFC 3782, page 6:
    // "6)  Retransmit timeouts:
    // After a retransmit timeout, record the highest sequence number
    // transmitted in the variable "recover" and exit the Fast Recovery
    // procedure if applicable."
    state->recover = (state->snd_max - 1);
    EV_INFO << "recover=" << state->recover << "\n";
    state->lossRecovery = false;
    state->firstPartialACK = false;
    EV_INFO << "Loss Recovery terminated.\n";

    // After REXMIT timeout TCP NewReno should start slow start with snd_cwnd = snd_mss.
    //
    // If calling "retransmitData();" there is no rexmit limitation (bytesToSend > snd_cwnd)
    // therefore "sendData();" has been modified and is called to rexmit outstanding data.
    //
    // RFC 2581, page 5:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."

    // RFC 2581, page 4:
    // "When a TCP sender detects segment loss using the retransmission
    // timer, the value of ssthresh MUST be set to no more than the value
    // given in equation 3:
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (3)
    //
    // As discussed above, FlightSize is the amount of outstanding data in
    // the network."
    state->ssthresh = std::max(conn->getBytesInFlight() / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";
    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

void TcpTahoeRenoFamily::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    if (recovery->isDuplicateAck(tcpHeader, payloadLength)) {
        if (!state->lossRecovery) {
            state->dupacks++;
            conn->emit(dupAcksSignal, state->dupacks);
        }
        receivedDuplicateAck();
    }
    else {
        // if doesn't qualify as duplicate ACK, just ignore it.
        if (payloadLength == 0) {
            if (state->snd_una != tcpHeader->getAckNo())
                EV_DETAIL << "Old ACK: ackNo < snd_una\n";
            else if (state->snd_una == state->snd_max)
                EV_DETAIL << "ACK looks duplicate but we have currently no unacked data (snd_una == snd_max)\n";
        }
        // reset counter
        state->dupacks = 0;
        conn->emit(dupAcksSignal, state->dupacks);
    }
}

void TcpTahoeRenoFamily::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);
    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    if (state->lossRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    if (!state->lossRecovery)
        congestionControl->receivedAckForUnackedData(numBytesAcked);
    sendData(false);
}

void TcpTahoeRenoFamily::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
}

} // namespace tcp
} // namespace inet

