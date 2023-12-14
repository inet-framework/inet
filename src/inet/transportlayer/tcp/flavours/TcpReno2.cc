//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpReno2.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpReno2);

ITcpRecovery *TcpReno2::createRecovery()
{
    return new Rfc5681Recovery(state, conn);
}

ITcpCongestionControl *TcpReno2::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

} // namespace tcp
} // namespace inet

