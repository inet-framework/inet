//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc5681.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

void Rfc5681::receivedDataAck(uint32_t firstSeqAcked)
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
            state->snd_cwnd += state->snd_mss; // TODO implement min function from (2)
// TODO           conn->emit(cwndSignal, state->snd_cwnd);
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
// TODO           conn->emit(cwndSignal, state->snd_cwnd);
        }
    }
}

void Rfc5681::receivedDuplicateAck()
{
    // 3.2. Fast Retransmit/Fast Recovery
    //
    // ...
    //
    // The fast retransmit and fast recovery algorithms are implemented
    // together as follows.
    //
    // 1. On the first and second duplicate ACKs received at a sender, a
    //    TCP SHOULD send a segment of previously unsent data per [RFC3042]
    //    provided that the receiver's advertised window allows, the total
    //    FlightSize would remain less than or equal to cwnd plus 2*SMSS,
    //    and that new data is available for transmission.  Further, the
    //    TCP sender MUST NOT change cwnd to reflect these two segments
    //    [RFC3042].  Note that a sender using SACK [RFC2018] MUST NOT send
    //    new data unless the incoming duplicate acknowledgment contains
    //    new SACK information.
    // TODO how is this implemented?

    // 2. When the third duplicate ACK is received, a TCP MUST set ssthresh
    //    to no more than the value given in equation (4).  When [RFC3042]
    //    is in use, additional data sent in limited transmit MUST NOT be
    //    included in this calculation.
    if (state->dupacks == state->dupthresh) {
        // When a TCP sender detects segment loss using the retransmission timer
        // and the given segment has not yet been resent by way of the
        // retransmission timer, the value of ssthresh MUST be set to no more
        // than the value given in equation (4):
        //
        //   ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
        //
        // where, as discussed above, FlightSize is the amount of outstanding
        // data in the network.
        state->ssthresh = std::max(conn->getBytesInFlight() / 2, 2 * state->snd_mss);
// TODO       conn->emit(ssthreshSignal, state->ssthresh);

        // 3. The lost segment starting at SND.UNA MUST be retransmitted and
        //    cwnd set to ssthresh plus 3*SMSS.  This artificially "inflates"
        //    the congestion window by the number of segments (three) that have
        //    left the network and which the receiver has buffered.
        conn->retransmitOneSegment(false);
        state->snd_cwnd = state->ssthresh + 3 * state->snd_mss;
// TODO        conn->emit(cwndSignal, state->snd_cwnd);
    }
    // 4. For each additional duplicate ACK received (after the third),
    //    cwnd MUST be incremented by SMSS.  This artificially inflates the
    //    congestion window in order to reflect the additional segment that
    //    has left the network.
    else if (state->dupacks > state->dupthresh) {
        state->snd_cwnd += state->snd_mss;
// TODO        conn->emit(cwndSignal, state->snd_cwnd);
    }
}

} // namespace tcp
} // namespace inet

