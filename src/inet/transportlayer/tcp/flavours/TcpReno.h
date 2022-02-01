//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPRENO_H
#define __INET_TCPRENO_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpReno.
 */
typedef TcpTahoeRenoFamilyStateVariables TcpRenoStateVariables;

/**
 * Implements TCP Reno.
 */
class INET_API TcpReno : public TcpTahoeRenoFamily
{
  protected:
    TcpRenoStateVariables *& state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TcpRenoStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpRenoStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    /** Ctor */
    TcpReno();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

