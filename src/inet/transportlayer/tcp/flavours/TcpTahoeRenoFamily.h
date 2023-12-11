//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamilyState_m.h"

namespace inet {
namespace tcp {

/**
 * Provides utility functions to implement TcpTahoe, TcpReno and TcpNewReno.
 * (TcpVegas should inherit from TcpAlgorithmBase instead of this one.)
 */
class INET_API TcpTahoeRenoFamily : public TcpAlgorithmBase
{
  protected:
    TcpTahoeRenoFamilyStateVariables *& state; // alias to TcpAlgorithm's 'state'

    ITcpCongestionControl *congestionControl = nullptr;
    ITcpRecovery *recovery = nullptr;

  protected:
    virtual ITcpRecovery *createRecovery() { return nullptr; }
    virtual ITcpCongestionControl *createCongestionControl() { return nullptr; }


    virtual void established(bool active) override;
  public:
    /** Ctor */
    TcpTahoeRenoFamily();

    virtual void initialize() override;

    virtual ITcpCongestionControl *getCongestionControl() { return congestionControl; }
    virtual ITcpRecovery *getRecovery() { return recovery; }

    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;
};

} // namespace tcp
} // namespace inet

#endif

