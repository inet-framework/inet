//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DUMBTCP_H
#define __INET_DUMBTCP_H

#include "inet/transportlayer/tcp/TcpAlgorithm.h"
#include "inet/transportlayer/tcp/flavours/DumbTcpState_m.h"

namespace inet {
namespace tcp {

/**
 * A very-very basic TcpAlgorithm implementation, with hardcoded
 * retransmission timeout and no other sophistication. It can be
 * used to demonstrate what happened if there was no adaptive
 * timeout calculation, delayed acks, silly window avoidance,
 * congestion control, etc.
 */
class INET_API DumbTcp : public TcpAlgorithm
{
  protected:
    DumbTcpStateVariables *& state; // alias to TCLAlgorithm's 'state'

    cMessage *rexmitTimer; // retransmission timer

  protected:
    /** Creates and returns a DumbTcpStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new DumbTcpStateVariables();
    }

  public:
    /** Ctor */
    DumbTcp();

    virtual ~DumbTcp();

    virtual void initialize() override;

    virtual void established(bool active) override;

    virtual void connectionClosed() override;

    virtual void processTimer(cMessage *timer, TcpEventCode& event) override;

    virtual void sendCommandInvoked() override;

    virtual void receivedOutOfOrderSegment() override;

    virtual void receiveSeqChanged() override;

    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;

    virtual void receivedAckForDataNotYetSent(uint32_t seq) override;

    virtual void ackSent() override;

    virtual void dataSent(uint32_t fromseq) override;

    virtual void segmentRetransmitted(uint32_t fromseq, uint32_t toseq) override;

    virtual void restartRexmitTimer() override;

    virtual void rttMeasurementCompleteUsingTS(uint32_t echoedTS) override;

    virtual bool shouldMarkAck() override;

    virtual void processEcnInEstablished() override;
};

} // namespace tcp
} // namespace inet

#endif

