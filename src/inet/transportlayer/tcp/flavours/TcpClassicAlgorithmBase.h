//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

/**
 * Provides utility functions to implement TcpTahoe, TcpReno and TcpNewReno.
 * (TcpVegas should inherit from TcpAlgorithmBase instead of this one.)
 */
class INET_API TcpClassicAlgorithmBase : public TcpAlgorithmBase
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'

  public:
    /** Ctor */
    TcpClassicAlgorithmBase();

    virtual void initialize() override;
};


// Deprecated: TcpTahoeRenoFamily was renamed to TcpClassicAlgorithmBase in INET 4.6, because the old name said
// what the class inherits rather than what it is -- it is the base of the classic loss-based algorithms.
// The alias keeps code outside this repository compiling for one release; it goes
// away in the release after that.
using TcpTahoeRenoFamily = TcpClassicAlgorithmBase;

} // namespace tcp
} // namespace inet

#endif

