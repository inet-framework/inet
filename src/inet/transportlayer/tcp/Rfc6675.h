//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC6675ALG_H
#define __INET_RFC6675ALG_H

#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 6675: A Conservative Loss Recovery Algorithm Based on Selective Acknowledgment (SACK) for TCP.
 */
class INET_API Rfc6675
{
  protected:
    TcpTahoeRenoFamilyStateVariables *state = nullptr;
    TcpConnection *conn = nullptr;

    virtual void stepA();
    virtual void stepB();
    virtual void stepC();

    virtual void step4();

  public:
    Rfc6675(TcpStateVariables *state, TcpConnection *conn) : state(check_and_cast<TcpTahoeRenoFamilyStateVariables *>(state)), conn(conn) { }

    virtual void receivedDataAck(uint32_t firstSeqAcked);

    virtual void receivedDuplicateAck();
};

} // namespace tcp
} // namespace inet

#endif

