//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpNewReno2.h"

namespace inet {
namespace tcp {

Register_Class(TcpNewReno2);

// 1) Initialization of TCP protocol control block:
//    When the TCP protocol control block is initialized, recover is
//    set to the initial send sequence number.

TcpNewReno2::TcpNewReno2() : TcpTahoeRenoFamily(),
    state((TcpNewReno2StateVariables *&)TcpAlgorithm::state)
{
}

void TcpNewReno2::processRexmitTimer(TcpEventCode& event)
{
    TcpTahoeRenoFamily::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // RFC 3782, page 6:
    // "6)  Retransmit timeouts:
    // After a retransmit timeout, record the highest sequence number
    // transmitted in the variable "recover" and exit the Fast Recovery
    // procedure if applicable."
    state->recover = (state->snd_max - 1);
    EV_INFO << "recover=" << state->recover << "\n";
    state->lossRecovery = false;
    state->firstPartialACK = false;
    EV_INFO << "Loss Recovery terminated.\n";

    // After REXMIT timeout TCP NewReno should start slow start with snd_cwnd = snd_mss.
    //
    // If calling "retransmitData();" there is no rexmit limitation (bytesToSend > snd_cwnd)
    // therefore "sendData();" has been modified and is called to rexmit outstanding data.
    //
    // RFC 2581, page 5:
    // "Furthermore, upon a timeout cwnd MUST be set to no more than the loss
    // window, LW, which equals 1 full-sized segment (regardless of the
    // value of IW).  Therefore, after retransmitting the dropped segment
    // the TCP sender uses the slow start algorithm to increase the window
    // from 1 full-sized segment to the new value of ssthresh, at which
    // point congestion avoidance again takes over."

    // RFC 2581, page 4:
    // "When a TCP sender detects segment loss using the retransmission
    // timer, the value of ssthresh MUST be set to no more than the value
    // given in equation 3:
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (3)
    //
    // As discussed above, FlightSize is the amount of outstanding data in
    // the network."
    state->ssthresh = std::max(conn->getBytesInFlight() / 2, 2 * state->snd_mss);
    conn->emit(ssthreshSignal, state->ssthresh);

    state->snd_cwnd = state->snd_mss;
    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
            << ", ssthresh=" << state->ssthresh << "\n";
    state->afterRto = true;
    conn->retransmitOneSegment(true);
}

void TcpNewReno2::receivedAckForUnackedData(uint32_t firstSeqAcked)
{
    TcpTahoeRenoFamily::receivedAckForUnackedData(firstSeqAcked);
    uint32_t numBytesAcked = state->snd_una - firstSeqAcked;
    recovery->receivedAckForUnackedData(numBytesAcked);
    congestionControl->receivedAckForUnackedData(numBytesAcked);
    sendData(false);
}

void TcpNewReno2::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
    congestionControl->receivedDuplicateAck();
}

} // namespace tcp
} // namespace inet

