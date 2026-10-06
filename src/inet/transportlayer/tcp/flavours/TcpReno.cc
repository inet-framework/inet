//
// Copyright (C) 2004-2005 OpenSim Ltd.
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpReno.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSimsignals.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpReno);

TcpReno::TcpReno() : TcpClassicAlgorithmBase(),
    state((TcpRenoStateVariables *&)TcpAlgorithm::state)
{
}

ITcpCongestionControl *TcpReno::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

ITcpRecovery *TcpReno::createRecovery()
{
    if (state->sack_enabled)
        return new Rfc6675Recovery(state, conn);
    else
        return new Rfc5681Recovery(state, conn);
}

} // namespace tcp
} // namespace inet

