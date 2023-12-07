//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPNEWRENO2_H
#define __INET_TCPNEWRENO2_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

typedef TcpTahoeRenoFamilyStateVariables TcpNewReno2StateVariables;

/**
 * Implements RFC 6582: The NewReno Modification to TCP's Fast Recovery Algorithm.
 */
class INET_API TcpNewReno2 : public TcpTahoeRenoFamily
{
  protected:
    TcpNewReno2StateVariables *& state;

    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpNewReno2StateVariables();
    }

    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    TcpNewReno2();

    virtual void receivedAckForDataNotYetAcked(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

