//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2009-2010 Thomas Reschka
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPSIGNALS_H
#define __INET_TCPSIGNALS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace tcp {

extern INET_API simsignal_t bytesInFlightSignal;
extern INET_API simsignal_t cwndSignal; // will record changes to snd_cwnd
extern INET_API simsignal_t cwndSignal; // will record changes to snd_cwnd
extern INET_API simsignal_t dupAcksSignal; // current number of received dupAcks
extern INET_API simsignal_t numRtosSignal; // will record total number of RTOs
extern INET_API simsignal_t numRtosSignal; // will record total number of RTOs
extern INET_API simsignal_t pipeSignal; // current sender's estimate of bytes outstanding in the network
extern INET_API simsignal_t rcvAckSignal; // received ackNo (=snd_una)
extern INET_API simsignal_t rcvAdvSignal; // current advertised window (=rcv_adv)
extern INET_API simsignal_t rcvNASegSignal; // number of received not acceptable segments
extern INET_API simsignal_t rcvOooSegSignal; // number of received out-of-order segments
extern INET_API simsignal_t rcvSacksSignal; // number of received Sacks
extern INET_API simsignal_t rcvSeqSignal; // received seqNo
extern INET_API simsignal_t rcvWndSignal; // rcv_wnd
extern INET_API simsignal_t rtoSignal; // will record retransmission timeout
extern INET_API simsignal_t rtoSignal; // will record retransmission timeout
extern INET_API simsignal_t rttSignal; // will record measured RTT
extern INET_API simsignal_t rttSignal; // will record measured RTT
extern INET_API simsignal_t rttvarSignal; // will record RTT variance (rttvar)
extern INET_API simsignal_t rttvarSignal; // will record RTT variance (rttvar)
extern INET_API simsignal_t sackedBytesSignal; // current number of received sacked bytes
extern INET_API simsignal_t sndAckSignal; // sent ackNo
extern INET_API simsignal_t sndMaxSignal; // snd_max
extern INET_API simsignal_t sndSacksSignal; // number of sent Sacks
extern INET_API simsignal_t sndSeqSignal; // sent seqNo
extern INET_API simsignal_t sndWndSignal; // snd_wnd
extern INET_API simsignal_t srttSignal; // will record smoothed RTT
extern INET_API simsignal_t srttSignal; // will record smoothed RTT
extern INET_API simsignal_t ssthreshSignal; // will record changes to ssthresh
extern INET_API simsignal_t ssthreshSignal; // will record changes to ssthresh
extern INET_API simsignal_t stateSignal; // FSM state
extern INET_API simsignal_t tcpConnectionAddedSignal;
extern INET_API simsignal_t tcpRcvPayloadBytesSignal; // amount of payload bytes received (including duplicates, out of order etc) for TCP throughput
extern INET_API simsignal_t tcpRcvQueueBytesSignal; // current amount of used bytes in tcp receive queue
extern INET_API simsignal_t tcpRcvQueueDropsSignal; // number of drops in tcp receive queue
extern INET_API simsignal_t unackedSignal; // number of bytes unacknowledged

} // namespace tcp
} // namespace inet

#endif

