//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPNEWRENO_H
#define __INET_TCPNEWRENO_H

#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 6582: The NewReno Modification to TCP's Fast Recovery Algorithm.
 */
class INET_API TcpNewReno : public TcpClassicAlgorithmBase
{
  protected:
    virtual ITcpRecovery *createRecovery() override;
    virtual ITcpCongestionControl *createCongestionControl() override;
};

} // namespace tcp
} // namespace inet

#endif

