//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC5681_H
#define __INET_RFC5681_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

typedef TcpTahoeRenoFamilyStateVariables Rfc5681StateVariables;

/**
 * Implements RFC 5681: TCP Congestion Control.
 */
class INET_API Rfc5681 : public TcpTahoeRenoFamily
{
  protected:
    Rfc5681StateVariables *& state;

    virtual TcpStateVariables *createStateVariables() override
    {
        return new Rfc5681StateVariables();
    }

    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    Rfc5681();

    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

