//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPFREEBSD_H
#define __INET_TCPFREEBSD_H

#include "inet/transportlayer/tcp/flavours/freebsd/cc.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"
#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBaseState_m.h"

namespace inet {
namespace tcp {

class INET_API TcpFreeBsdAlgorithm : public TcpAlgorithmBase
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *& state; // alias to TcpAlgorithm's 'state'
    cc_var cc;
    cc_algo algo;

  protected:
    virtual TcpStateVariables *createStateVariables() override { return new TcpClassicAlgorithmBaseStateVariables(); }

    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) override;

    void copythere();
    void copyback();

  public:
    TcpFreeBsdAlgorithm();

    virtual void initialize() override;
    virtual void established(bool active) override;
//    virtual void processRexmitTimer(TcpEventCode& event) override;
    virtual void receivedAckForAlreadyAckedData(const TcpHeader *tcpHeader, uint32_t payloadLength) override;
    virtual void receivedAckForUnackedData(uint32_t firstSeqAcked) override;
//    virtual void receivedAckForUnsentData(uint32_t seq) override;
    virtual void receivedDuplicateAck() override;
    virtual uint32_t getBytesInFlight() const override;
};

} // namespace tcp
} // namespace inet

#endif // __INET_TCPFREEBSD_H
