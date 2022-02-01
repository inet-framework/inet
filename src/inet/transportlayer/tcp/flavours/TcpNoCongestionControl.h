//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPNOCONGESTIONCONTROL_H
#define __INET_TCPNOCONGESTIONCONTROL_H

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpNoCongestionControl.
 */
typedef TcpBaseAlgStateVariables TcpNoCongestionControlStateVariables;

/**
 * TCP with no congestion control (i.e. congestion window kept very large).
 * Can be used to demonstrate effect of lack of congestion control.
 */
class INET_API TcpNoCongestionControl : public TcpBaseAlg
{
  protected:
    TcpNoCongestionControlStateVariables *& state; // alias to TcpAlgorithm's 'state'

    /** Create and return a TcpNoCongestionControlStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpNoCongestionControlStateVariables();
    }

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    /** Ctor */
    TcpNoCongestionControl();

    /** Initialize state vars */
    virtual void initialize() override;

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual void established(bool active) override;

    virtual bool sendData(bool sendCommandInvoked) override;
};

} // namespace tcp
} // namespace inet

#endif

