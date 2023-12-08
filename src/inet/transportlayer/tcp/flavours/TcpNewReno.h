//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPNEWRENO_H
#define __INET_TCPNEWRENO_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

typedef TcpTahoeRenoFamilyStateVariables TcpNewRenoStateVariables;

/**
 * Implements RFC 6582: The NewReno Modification to TCP's Fast Recovery Algorithm.
 */
class INET_API TcpNewReno : public TcpTahoeRenoFamily
{
  protected:
    TcpNewRenoStateVariables *& state;

    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpNewRenoStateVariables();
    }

    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    TcpNewReno();

    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

