//
// Copyright (C) 2013 Maria Fernandez, Carlos Calafate, Juan-Carlos Cano and Pietro Manzoni
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPWESTWOOD_H
#define __INET_TCPWESTWOOD_H

#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"
#include "inet/transportlayer/tcp/flavours/TcpWestwoodState_m.h"

namespace inet {
namespace tcp {

class INET_API TcpWestwood : public TcpBaseAlg
{
  protected:
    TcpWestwoodStateVariables *& state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TCPvegasStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpWestwoodStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

    /** Recalculate BWE */
    virtual void recalculateBWE(uint32_t cumul_ack);

  public:
    /** Ctor */
    TcpWestwood();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;

    /** Called after we send data */
    virtual void dataSent(uint32_t fromseq) override;

    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) override;
};

} // namespace tcp
} // namespace inet

#endif

