//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"

namespace inet {
namespace tcp {

void Rfc5681Recovery::receivedDataAck(uint32_t firstSeqAcked)
{
}

void Rfc5681Recovery::receivedDuplicateAck()
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

