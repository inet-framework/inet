//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ITCPRECOVERY_H
#define __INET_ITCPRECOVERY_H

#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

class INET_API ITcpRecovery : public cObject
{
  public:
    virtual bool isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength) = 0;

    virtual void receivedAckForUnackedData(uint32_t numBytesAcked) = 0;

    virtual void receivedDuplicateAck() = 0;

    /**
     * Called when snd_una is about to advance, BEFORE the acked range
     * [fromSeq, toSeq) is discarded from the send/rexmit queues, while the
     * scoreboard entry for that range (transmit count, SACK state) is still
     * valid. Lets a recovery algorithm tell reordering apart from loss.
     */
    virtual void segmentsAcked(uint32_t fromSeq, uint32_t toSeq) {}

    /**
     * Called after data was sent; the argument is the seqno of the first byte.
     * Used by rate-limited recovery (RFC 6937 PRR) to account transmitted
     * bytes, and by loss probes to (re)arm their timer.
     */
    virtual void dataSent(uint32_t fromSeq) {}

    /**
     * Called after a segment was retransmitted, over [fromSeq, toSeq).
     */
    virtual void segmentRetransmitted(uint32_t fromSeq, uint32_t toSeq) {}

    /**
     * Called when the retransmission timer expired, after the algorithm's own
     * RTO handling. Lets a recovery algorithm capture undo state or start a
     * spurious-RTO detection episode (RFC 5682 F-RTO).
     */
    virtual void onRexmitTimeout() {}

    /**
     * Called when the RACK reordering timer expired (RFC 8985 / Linux
     * ICSK_TIME_REO_TIMEOUT) and loss detection has just marked further bytes
     * lost, so the newly lost data can be retransmitted.
     */
    virtual void reoTimeout() {}
};

} // namespace tcp
} // namespace inet

#endif

