//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/Rfc5681Recovery.h"

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "inet/transportlayer/tcp/TcpSackRexmitQueue.h"
#include "inet/transportlayer/tcp/TcpSendQueue.h"

namespace inet {
namespace tcp {

bool Rfc5681Recovery::isDuplicateAck(const TcpHeader *tcpHeader, uint32_t payloadLength)
{
    //"
    // DUPLICATE ACKNOWLEDGMENT: An acknowledgment is considered a
    // "duplicate" in the following algorithms when
    //   (a) the receiver of the ACK has outstanding data,
    //"
    bool a = state->snd_una != state->snd_max;
    //"
    //   (b) the incoming acknowledgment carries no data,
    //"
    bool b = payloadLength == 0;
    //"
    //   (c) the SYN and FIN bits are both off,
    //"
    bool c = !tcpHeader->getSynBit() && !tcpHeader->getFinBit();
    //"
    //   (d) the acknowledgment number is equal to the greatest acknowledgment
    //       received on the given connection (TCP.UNA from [RFC 793]) and
    //"
    bool d = tcpHeader->getAckNo() == state->snd_una;
    //"
    //   (e) the advertised window in the incoming acknowledgment equals the
    //       advertised window in the last incoming acknowledgment.
    //"
    uint32_t trueWindow = tcpHeader->getWindow();
    if (state->ws_enabled && !tcpHeader->getSynBit())
        trueWindow = tcpHeader->getWindow() << state->snd_wnd_scale;
    bool e = trueWindow == state->snd_wnd;
    return a && b && c && d && e;
}

void Rfc5681Recovery::receivedAckForUnackedData(uint32_t numBytesAcked)
{
    ASSERT(state->lossRecovery);
    //"
    // 6. When the next ACK arrives that acknowledges previously
    //    unacknowledged data, a TCP MUST set cwnd to ssthresh (the value
    //    set in step 2).  This is termed "deflating" the window.
    //
    //    This ACK should be the acknowledgment elicited by the
    //    retransmission from step 3, one RTT after the retransmission
    //    (though it may arrive sooner in the presence of significant out-
    //    of-order delivery of data segments at the receiver).
    //    Additionally, this ACK should acknowledge all the intermediate
    //    segments sent between the lost segment and the receipt of the
    //    third duplicate ACK, if none of these were lost.
    //"
    state->snd_cwnd = state->ssthresh;
    conn->emit(cwndSignal, state->snd_cwnd);
    state->lossRecovery = false;
    EV_INFO << "Loss recovery terminated" << EV_ENDL;
}

void Rfc5681Recovery::receivedDuplicateAck()
{
    //"
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
    //"
    if (state->dupacks < state->dupthresh)
        // TODO FlightSize would remain less than or equal to cwnd plus 2*SMSS
        conn->sendData(state->snd_cwnd);
    //"
    // 2. When the third duplicate ACK is received, a TCP MUST set ssthresh
    //    to no more than the value given in equation (4).  When [RFC3042]
    //    is in use, additional data sent in limited transmit MUST NOT be
    //    included in this calculation.
    //"
    // dupacks is frozen for the duration of the recovery phase, so every further
    // duplicate ACK arrives with dupacks == dupthresh; only the first one may enter
    // fast retransmit, the rest fall through to step 5's send below
    else if (state->dupacks == state->dupthresh && !state->lossRecovery) {
        //"
        // When a TCP sender detects segment loss using the retransmission timer
        // and the given segment has not yet been resent by way of the
        // retransmission timer, the value of ssthresh MUST be set to no more
        // than the value given in equation (4):
        //
        //   ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
        //
        // where, as discussed above, FlightSize is the amount of outstanding
        // data in the network.
        //"
        uint32_t flightSize = conn->getTcpAlgorithm()->getBytesInFlight() + state->snd_effmss; // the +1 MSS accounts for the retransmitOneSegment call below
        state->ssthresh = conn->getTcpAlgorithmForUpdate()->calculateSsthresh(flightSize);
        conn->emit(ssthreshSignal, state->ssthresh);

        //"
        // 3. The lost segment starting at SND.UNA MUST be retransmitted and
        //    cwnd set to ssthresh plus 3*SMSS.  This artificially "inflates"
        //    the congestion window by the number of segments (three) that have
        //    left the network and which the receiver has buffered.
        //"
        conn->retransmitOneSegment(false);
        state->snd_cwnd = state->ssthresh; // no +3*SMSS inflation: getBytesInFlight already accounts for the 3 segments in sackedOut
        conn->emit(cwndSignal, state->snd_cwnd);

        // entering fast retransmit means starting the loss recovery phase; the ACK
        // that ends it runs step 6 in receivedAckForUnackedData()
        state->lossRecovery = true;
    }
    //"
    // 4. For each additional duplicate ACK received (after the third),
    //    cwnd MUST be incremented by SMSS.  This artificially inflates the
    //    congestion window in order to reflect the additional segment that
    //    has left the network.
    //"
    // "additional" is counted by arrival, not by state->dupacks: the counter is frozen
    // at dupthresh for the whole recovery phase, so every further duplicate ACK inside
    // it is an additional one
    else if (state->dupacks > state->dupthresh || state->lossRecovery) {
        state->snd_cwnd += state->snd_effmss;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    //"
    // 5.  When previously unsent data is available and the new value of
    //     cwnd and the receiver's advertised window allow, a TCP SHOULD
    //     send 1*SMSS bytes of previously unsent data.
    //"
    conn->sendData(state->snd_cwnd);
}

} // namespace tcp
} // namespace inet

