//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamilyState_m.h"

namespace inet {
namespace tcp {

/**
 * Provides utility functions to implement TcpTahoe, TcpReno and TcpNewReno.
 * (TcpVegas should inherit from TcpBaseAlg instead of this one.)
 */
class INET_API TcpTahoeRenoFamily : public TcpBaseAlg
{
  protected:
    TcpTahoeRenoFamilyStateVariables *& state; // alias to TcpAlgorithm's 'state'

  public:
    /** Ctor */
    TcpTahoeRenoFamily();

    virtual void initialize() override;
};

} // namespace tcp
} // namespace inet

#endif

