//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPTAHOE_H
#define __INET_TCPTAHOE_H

#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpTahoe.
 */
typedef TcpTahoeRenoFamilyStateVariables TcpTahoeStateVariables;

/**
 * Implements Tahoe.
 */
class INET_API TcpTahoe : public TcpTahoeRenoFamily
{
  protected:
    TcpTahoeStateVariables *& state; // alias to TCLAlgorithm's 'state'

  protected:
    /** Create and return a TcpTahoeStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpTahoeStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    /** Ctor */
    TcpTahoe();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;
};

} // namespace tcp
} // namespace inet

#endif

