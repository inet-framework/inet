//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RFC5681RECOVERY_H
#define __INET_RFC5681RECOVERY_H

#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp/ITcpRecovery.h"

namespace inet {
namespace tcp {

/**
 * Implements RFC 5681: TCP Congestion Control.
 */
class INET_API Rfc5681Recovery : public ITcpRecovery
{
  protected:
    TcpClassicAlgorithmBaseStateVariables *state = nullptr;
    TcpConnection *conn = nullptr;
    // the guard of RFC 6582 section 6 after a timeout, for TcpReno; TcpNewReno runs the
    // steps of RFC 5681 through this class too, and keeps recover itself
    bool recoverAfterTimeout = false;

  public:
    Rfc5681Recovery(TcpStateVariables *state, TcpConnection *conn, bool recoverAfterTimeout = false);

    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) override;

    virtual void receivedAckForUnackedData(uint32_t numBytesAcked) override;

    virtual void receivedDuplicateAck() override;

    // RFC 5681 recovery keeps no scoreboard and runs no timer of its own, so
    // four scoreboard and timer hooks of ITcpRecovery do nothing here; a timeout
    // sets recover (RFC 6582 section 6).
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) override {}
    virtual void dataSent(uint32_t fromSeq) override {}
    virtual void segmentRetransmitted(uint32_t fromSeq, uint32_t toSeq) override {}
    virtual void onRexmitTimeout() override;
    virtual void reoTimeout() override {}
};

} // namespace tcp
} // namespace inet

#endif

