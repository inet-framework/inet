//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOEWITHOUTFASTRETRANSMIT_H
#define __INET_TCPTAHOEWITHOUTFASTRETRANSMIT_H

#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

/**
 * This class serves educational purposes to demonstrate a simple congestion
 * control algorithm. It only implements slow start and congestion avoidance
 * algorithms.
 */
class INET_API TcpTahoeWithoutFastRetransmit : public TcpAlgorithmBase
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *& state;

  protected:
    virtual TcpStateVariables *createStateVariables() override { return new TcpClassicAlgorithmBaseStateVariables(); }

    virtual void initialize() override;

    virtual void processRexmitTimer(TcpEventCode& event) override;

    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;

  public:
    TcpTahoeWithoutFastRetransmit();
};

} // namespace tcp
} // namespace inet

#endif

