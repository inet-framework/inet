//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpTahoeRenoFamily.
 */
class INET_API TcpTahoeRenoFamilyStateVariables : public TcpBaseAlgStateVariables
{
  public:
    TcpTahoeRenoFamilyStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
    virtual void setSendQueueLimit(uint32_t newLimit);

    uint32_t ssthresh; ///< slow start threshold
};

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

    void initialize() override;
};

} // namespace tcp
} // namespace inet

#endif

