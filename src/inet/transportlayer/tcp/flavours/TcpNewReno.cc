//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpNewReno.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc6582Recovery.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpNewReno);

ITcpRecovery *TcpNewReno::createRecovery()
{
    if (state->sack_enabled)
        return new Rfc6675Recovery(state, conn);
    else
        return new Rfc6582Recovery(state, conn);
}

ITcpCongestionControl *TcpNewReno::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

} // namespace tcp
} // namespace inet

