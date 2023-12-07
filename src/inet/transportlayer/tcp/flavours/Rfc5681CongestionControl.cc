//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc5681CongestionControl.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

void Rfc5681CongestionControl::receivedAckForDataNotYetAcked(uint32_t numBytesAcked)
{
    if (!state->lossRecovery) {
        // 3.1. Slow Start and Congestion Avoidance
        if (state->snd_cwnd < state->ssthresh) {
            // During slow start, a TCP increments cwnd by at most SMSS bytes for
            // each ACK received that cumulatively acknowledges new data.  Slow
            // start ends when cwnd exceeds ssthresh (or, optionally, when it
            // reaches it, as noted above) or when congestion is observed.  While
            // traditionally TCP implementations have increased cwnd by precisely
            // SMSS bytes upon receipt of an ACK covering new data, we RECOMMEND
            // that TCP implementations increase cwnd, per:
            //
            //   cwnd += min (N, SMSS)                      (2)
            //
            // where N is the number of previously unacknowledged bytes acknowledged
            // in the incoming ACK.  This adjustment is part of Appropriate Byte
            // Counting [RFC3465] and provides robustness against misbehaving
            // receivers that may attempt to induce a sender to artificially inflate
            // cwnd using a mechanism known as "ACK Division" [SCWA99].  ACK
            // Division consists of a receiver sending multiple ACKs for a single
            // TCP data segment, each acknowledging only a portion of its data.  A
            // TCP that increments cwnd by SMSS for each such ACK will
            // inappropriately inflate the amount of data injected into the network.
            state->snd_cwnd += std::min(numBytesAcked, state->snd_mss);
            conn->emit(cwndSignal, state->snd_cwnd);
        }
        else {
            // The RECOMMENDED way to increase cwnd during congestion avoidance is
            // to count the number of bytes that have been acknowledged by ACKs for
            // new data.  (A drawback of this implementation is that it requires
            // maintaining an additional state variable.)  When the number of bytes
            // acknowledged reaches cwnd, then cwnd can be incremented by up to SMSS
            // bytes.  Note that during congestion avoidance, cwnd MUST NOT be
            // increased by more than SMSS bytes per RTT.  This method both allows
            // TCPs to increase cwnd by one segment per RTT in the face of delayed
            // ACKs and provides robustness against ACK Division attacks.
            //
            // Another common formula that a TCP MAY use to update cwnd during
            // congestion avoidance is given in equation (3):
            //
            //   cwnd += SMSS*SMSS/cwnd                     (3)
            //
            // This adjustment is executed on every incoming ACK that acknowledges
            // new data.  Equation (3) provides an acceptable approximation to the
            // underlying principle of increasing cwnd by 1 full-sized segment per
            // RTT.  (Note that for a connection in which the receiver is
            // acknowledging every-other packet, (3) is less aggressive than allowed
            // -- roughly increasing cwnd every second RTT.)
            //
            // Implementation Note: Since integer arithmetic is usually used in TCP
            // implementations, the formula given in equation (3) can fail to
            // increase cwnd when the congestion window is larger than SMSS*SMSS.
            // If the above formula yields 0, the result SHOULD be rounded up to 1
            // byte.
            uint32_t i = state->snd_mss * state->snd_mss / state->snd_cwnd;
            if (i == 0) i = 1;
            state->snd_cwnd += i;
            conn->emit(cwndSignal, state->snd_cwnd);
        }
    }
}

void Rfc5681CongestionControl::receivedDuplicateAck()
{
}

} // namespace tcp
} // namespace inet

