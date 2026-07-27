//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpReno.h"

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"
#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"
#include "inet/transportlayer/tcp/flavours/Rfc6675Recovery.h"

namespace inet {
namespace tcp {

Register_Class(TcpReno);

ITcpRecovery *TcpReno::createRecovery()
{
    // SACK is orthogonal to the congestion control flavour: when the connection
    // negotiated SACK, loss recovery must be the RFC 6675 scoreboard-based one,
    // because the SACK receive path (processSACKOption/addSacks) requires an
    // Rfc6675Recovery. TcpNewReno already selects this way; without it, enabling
    // sackSupport on a TcpReno connection aborts on the first SACK block.
    if (state->sack_enabled)
        return new Rfc6675Recovery(state, conn);
    else
        return new Rfc5681Recovery(state, conn);
}

ITcpCongestionControl *TcpReno::createCongestionControl()
{
    return new Rfc5681CongestionControl(state, conn);
}

} // namespace tcp
} // namespace inet

