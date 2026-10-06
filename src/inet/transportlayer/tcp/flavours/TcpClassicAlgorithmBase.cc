//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {
namespace tcp {

void TcpClassicAlgorithmBaseStateVariables::setSendQueueLimit(uint32_t newLimit)
{
    // The initial value of ssthresh SHOULD be set arbitrarily high (e.g.,
    // to the size of the largest possible advertised window) -> defined by sendQueueLimit
    sendQueueLimit = newLimit;
    ssthresh = sendQueueLimit;
}

std::string TcpClassicAlgorithmBaseStateVariables::str() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpClassicAlgorithmBaseStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpAlgorithmBaseStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

// ---

TcpClassicAlgorithmBase::TcpClassicAlgorithmBase() : TcpAlgorithmBase(),
    state((TcpClassicAlgorithmBaseStateVariables *&)TcpAlgorithm::state)
{
}

void TcpClassicAlgorithmBase::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

TcpClassicAlgorithmBase::~TcpClassicAlgorithmBase()
{
    delete congestionControl;
    delete recovery;
}

void TcpClassicAlgorithmBase::established(bool active)
{
    TcpAlgorithmBase::established(active);

    recovery = createRecovery();
    congestionControl = createCongestionControl();
}

void TcpClassicAlgorithmBase::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    if (recovery != nullptr)
        recovery->onRexmitTimeout();

    // RFC 5681, page 8:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."
    //
    // RFC 5681, page 7:
    // "When a TCP sender detects segment loss using the retransmission
    // timer and the given segment has not yet been resent by way of the
    // retransmission timer, the value of ssthresh MUST be set to no more
    // than the value given in equation (4):
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
    //
    // where, as discussed above, FlightSize is the amount of outstanding
    // data in the network."
    state->ssthresh = calculateSsthreshForRto();
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;

    conn->retransmitOneSegment(true);
}

void TcpClassicAlgorithmBase::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);

    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    bool inFastRecovery = isInFastRecovery();
    if (inFastRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    else if (!processEce())
        congestionControl->receivedAckForUnackedData(numBytesAcked);

    ackProcessed(inFastRecovery);

    sendData(false);
}

bool TcpClassicAlgorithmBase::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    return recovery->isDuplicateAck(tcpHeader, payloadLength);
}

void TcpClassicAlgorithmBase::receivedDuplicateAck()
{
    TcpAlgorithmBase::receivedDuplicateAck();

    recovery->receivedDuplicateAck();
}

} // namespace tcp
} // namespace inet

