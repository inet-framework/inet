//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC6582RECOVERY_H
#define __INET_RFC6582RECOVERY_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/ITcpRecovery.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 6582: The NewReno Modification to TCP's Fast Recovery Algorithm.
 */
class INET_API Rfc6582Recovery : public ITcpRecovery
{
  protected:
    TcpTahoeRenoFamilyStateVariables *state = nullptr;
    TcpConnection *conn = nullptr;

  public:
    Rfc6582Recovery(TcpStateVariables *state, TcpConnection *conn) : state(check_and_cast<TcpTahoeRenoFamilyStateVariables *>(state)), conn(conn) { }

    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForDataNotYetAcked(uint32_t numBytesAcked) override;

    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

