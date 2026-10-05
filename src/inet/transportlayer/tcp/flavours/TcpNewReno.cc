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
    return new Rfc6582Recovery(state, conn);
}

void TcpNewReno::ackProcessed(bool inFastRecovery)
{
    // outside fast recovery, "recover" trails snd_una
    if (!inFastRecovery)
        state->recover = (state->snd_una - 2);
}

} // namespace tcp
} // namespace inet

