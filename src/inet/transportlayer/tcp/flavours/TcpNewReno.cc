//
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpNewReno.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpNewReno);

TcpNewReno::TcpNewReno() : TcpClassicAlgorithmBase(),
    state((TcpNewRenoStateVariables *&)TcpAlgorithm::state)
{
}

ITcpCongestionControl *TcpNewReno::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

ITcpRecovery *TcpNewReno::createRecovery()
{
    if (state->sack_enabled)
        return new Rfc6675Recovery(state, conn);
    else
        return new Rfc6582Recovery(state, conn);
}

void TcpNewReno::ackProcessed(bool inFastRecovery)
{
    // Outside fast recovery, "recover" follows snd_una once the cumulative ACK
    // covers more than "recover". Before that, "recover" keeps the value of the
    // last fast retransmit or timeout, as RFC 6582 says; after that, the check
    // of step 2 holds either way, and "recover" cannot fall 2^31 behind snd_una.
    if (!inFastRecovery && seqGreater(state->snd_una - 1, state->recover))
        state->recover = (state->snd_una - 2);
}

} // namespace tcp
} // namespace inet

