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

} // namespace tcp
} // namespace inet

