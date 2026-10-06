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

bool TcpClassicAlgorithmBase::processEce()
{
    if (state->ect && state->gotEce) {
        // RFC 3168, page 18
        // "If the sender receives an ECN-Echo (ECE) ACK
        // packet (that is, an ACK packet with the ECN-Echo flag set in the TCP
        // header), then the sender knows that congestion was encountered in the
        // network on the path from the sender to the receiver.  The indication
        // of congestion should be treated just as a congestion loss in non-
        // ECN-Capable TCP. That is, the TCP source halves the congestion window
        // "cwnd" and reduces the slow start threshold "ssthresh".  The sending
        // TCP SHOULD NOT increase the congestion window in response to the
        // receipt of an ECN-Echo ACK packet.
        // ...
        //   The value of the congestion window is bounded below by a value of one MSS.
        // ...
        //   TCP should not react to congestion indications more than once every
        // window of data (or more loosely, more than once every round-trip
        // time). That is, the TCP sender's congestion window should be reduced
        // only once in response to a series of dropped and/or CE packets from a
        // single window of data.  In addition, the TCP source should not decrease
        // the slow-start threshold, ssthresh, if it has been decreased
        // within the last round trip time."
        if (simTime() - state->eceReactionTime > state->srtt) {
            state->snd_cwnd = std::max(state->snd_cwnd / 2, state->snd_mss);
            conn->emit(cwndSignal, state->snd_cwnd);
            EV_INFO << "cwnd = cwnd / 2: received ECN-Echo ACK... new cwnd = " << state->snd_cwnd << "\n";

            state->ssthresh = state->snd_cwnd;
            conn->emit(ssthreshSignal, state->ssthresh);
            EV_INFO << "ssthresh = cwnd: received ECN-Echo ACK... new ssthresh = " << state->ssthresh << "\n";

            state->sndCwr = true;

            // RFC 3168, page 18
            // "The sending TCP MUST reset the retransmit timer on receiving
            // the ECN-Echo packet when the congestion window is one."
            if (state->snd_cwnd == state->snd_mss) {
                restartRexmitTimer();
                EV_INFO << "cwnd = 1 MSS... reset retransmit timer.\n";
            }
            state->eceReactionTime = simTime();
            state->gotEce = false;
            return true;
        }
        else
            EV_INFO << "multiple ECN-Echo ACKs in less than rtt... no ECN reaction\n";
        state->gotEce = false;
    }
    return false;
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

