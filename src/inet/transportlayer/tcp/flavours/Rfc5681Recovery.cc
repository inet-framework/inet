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
#include "inet/transportlayer/tcp/TcpSimsignals.h"
#include "inet/transportlayer/tcp/flavours/TcpAlgorithmBase.h"

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
    //"
    // 6. When the next ACK arrives that acknowledges previously
    //    unacknowledged data, a TCP MUST set cwnd to ssthresh (the value
    //    set in step 2).  This is termed "deflating" the window.
    //"
    EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
    state->snd_cwnd = state->ssthresh;
    conn->emit(cwndSignal, state->snd_cwnd);
    state->lossRecovery = false;
}

void Rfc5681Recovery::receivedDuplicateAck()
{
    TcpAlgorithmBase *algorithm = check_and_cast<TcpAlgorithmBase *>(conn->getTcpAlgorithmForUpdate());

    //"
    // 2. When the third duplicate ACK is received, a TCP MUST set ssthresh
    //    to no more than the value given in equation (4).
    //"
    if (state->dupacks == state->dupthresh && !state->lossRecovery) {
        EV_INFO << "Reno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": perform Fast Retransmit, and enter Fast Recovery:";

        //"
        //   ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
        //
        // When [RFC3042] is in use, additional data sent in limited transmit
        // MUST NOT be included in this calculation.
        //"
        state->ssthresh = algorithm->calculateSsthresh(conn->getFlightSize());
        conn->emit(ssthreshSignal, state->ssthresh);

        //"
        // 3. The lost segment starting at SND.UNA MUST be retransmitted and
        //    cwnd set to ssthresh plus 3*SMSS.  This artificially "inflates"
        //    the congestion window by the number of segments (three) that have
        //    left the network and which the receiver has buffered.
        //"
        state->snd_cwnd = state->ssthresh + 3 * state->snd_mss;
        conn->emit(cwndSignal, state->snd_cwnd);

        EV_DETAIL << " set cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";

        conn->retransmitOneSegment(false);

        // the fast recovery lasts until an ACK of new data (step 6) or a timeout
        state->lossRecovery = true;

        // try to transmit new segments (RFC 2581)
        algorithm->sendData(false);
    }
    //"
    // 4. For each additional duplicate ACK received (after the third),
    //    cwnd MUST be incremented by SMSS.  This artificially inflates the
    //    congestion window in order to reflect the additional segment that
    //    has left the network.
    //"
    // In fast recovery every duplicate ACK is an additional one, also when an old ACK
    // that is no duplicate (for example data of the peer) has reset the counter.
    else if (state->lossRecovery) {
        state->snd_cwnd += state->snd_mss;
        EV_DETAIL << "Reno on dupAcks > DUPTHRESH(=" << state->dupthresh << ": Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";
        conn->emit(cwndSignal, state->snd_cwnd);

        //"
        // 5.  When previously unsent data is available and the new value of
        //     cwnd and the receiver's advertised window allow, a TCP SHOULD
        //     send 1*SMSS bytes of previously unsent data.
        //"
        algorithm->sendData(false);
    }
}

} // namespace tcp
} // namespace inet

