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
    out << TcpBaseAlgStateVariables::str();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TcpTahoeRenoFamilyStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TcpBaseAlgStateVariables::detailedInfo();
    out << "ssthresh=" << ssthresh << "\n";
    return out.str();
}

// ---

TcpTahoeRenoFamily::TcpTahoeRenoFamily() : TcpBaseAlg(),
    state((TcpTahoeRenoFamilyStateVariables *&)TcpAlgorithm::state)
{
}

void TcpTahoeRenoFamily::initialize()
{
    TcpBaseAlg::initialize();
    state->ssthresh = conn->getTcpMain()->par("initialSsthresh");
}

} // namespace tcp
} // namespace inet

