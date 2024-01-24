//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

void TcpClassicAlgorithmBaseStateVariables::setSendQueueLimit(uint32_t newLimit) {
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

TcpClassicAlgorithmBase::~TcpClassicAlgorithmBase()
{
    delete congestionControl;
    delete recovery;
}

void TcpClassicAlgorithmBase::initialize()
{
    TcpAlgorithmBase::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

void TcpClassicAlgorithmBase::established(bool active)
{
    TcpAlgorithmBase::established(active);

    recovery = createRecovery();
    congestionControl = createCongestionControl();
}

uint32_t TcpClassicAlgorithmBase::getBytesInFlight() const
{
    auto rexmitQueue = conn->getRexmitQueue();
    int64_t sentSize = state->snd_max - state->snd_una;
    int64_t in_flight = sentSize - rexmitQueue->getSacked() - rexmitQueue->getLost() + rexmitQueue->getRetrans();
    if (in_flight < 0)
        in_flight = 0;
    conn->emit(bytesInFlightSignal, in_flight);
    return in_flight;
}

void TcpClassicAlgorithmBase::processRexmitTimer(TcpEventCode& event)
{
    TcpAlgorithmBase::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // RFC 6582, page 6:
    // "4)  Retransmit timeouts:
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
    // RFC 5681, page 8:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."

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
    state->ssthresh = std::max(conn->getTcpAlgorithm()->getBytesInFlight() / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";
    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

void TcpClassicAlgorithmBase::receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength)
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

void TcpClassicAlgorithmBase::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpAlgorithmBase::receivedAckForUnackedData(firstSeqAcked);
    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    if (state->lossRecovery)
        recovery->receivedAckForUnackedData(numBytesAcked);
    if (!state->lossRecovery)
        congestionControl->receivedAckForUnackedData(numBytesAcked);
    sendData(false);
}

void TcpClassicAlgorithmBase::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
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
        }
        else
            EV_INFO << "multiple ECN-Echo ACKs in less than rtt... no ECN reaction\n";
        state->gotEce = false;
        return true;
    }
    else
        return false;
}

} // namespace tcp
} // namespace inet

