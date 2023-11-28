//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC5681RECOVERY_H
#define __INET_RFC5681RECOVERY_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/TcpRecovery.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 5681: TCP Congestion Control.
 */
class INET_API Rfc5681Recovery : public TcpRecovery
{
  protected:
    TcpTahoeRenoFamilyStateVariables *state = nullptr;
    TcpConnection *conn = nullptr;

  public:
    Rfc5681Recovery(TcpStateVariables *state, TcpConnection *conn) : state(check_and_cast<TcpTahoeRenoFamilyStateVariables *>(state)), conn(conn) { }

    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

