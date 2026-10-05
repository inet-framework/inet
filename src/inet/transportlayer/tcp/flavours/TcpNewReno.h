//
// Copyright (C) 2009 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPNEWRENO_H
#define __INET_TCPNEWRENO_H

#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpNewReno.
 */
typedef TcpClassicAlgorithmBaseStateVariables TcpNewRenoStateVariables;

/**
 * Implements TCP NewReno.
 */
class INET_API TcpNewReno : public TcpClassicAlgorithmBase
{
  protected:
    TcpNewRenoStateVariables *& state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TcpNewRenoStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpNewRenoStateVariables();
    }

    virtual ITcpCongestionControl *createCongestionControl() override;
    virtual ITcpRecovery *createRecovery() override;

    virtual void ackProcessed(bool inFastRecovery) override;

  public:
    /** Ctor */
    TcpNewReno();
};

} // namespace tcp
} // namespace inet

#endif

