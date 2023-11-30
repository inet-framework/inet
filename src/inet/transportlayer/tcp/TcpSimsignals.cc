//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/TcpSimsignals.h"

namespace inet {
namespace tcp {

simsignal_t bytesInFlightSignal = cComponent::registerSignal("bytesInFlight"); // amount of payload bytes received (including duplicates, out of order etc) for TCP throughput
simsignal_t cwndSignal = cComponent::registerSignal("cwnd"); // will record changes to snd_cwnd
simsignal_t dupAcksSignal = cComponent::registerSignal("dupAcks"); // current number of received dupAcks
simsignal_t numRtosSignal = cComponent::registerSignal("numRtos"); // will record total number of RTOs
simsignal_t pipeSignal = cComponent::registerSignal("pipe"); // current sender's estimate of bytes outstanding in the network
simsignal_t rcvAckSignal = cComponent::registerSignal("rcvAck"); // received ackNo (=snd_una)
simsignal_t rcvAdvSignal = cComponent::registerSignal("rcvAdv"); // current advertised window (=rcv_adv)
simsignal_t rcvNASegSignal = cComponent::registerSignal("rcvNASeg"); // number of received not acceptable segments
simsignal_t rcvOooSegSignal = cComponent::registerSignal("rcvOooSeg"); // number of received out-of-order segments
simsignal_t rcvSacksSignal = cComponent::registerSignal("rcvSacks"); // number of received Sacks
simsignal_t rcvSeqSignal = cComponent::registerSignal("rcvSeq"); // received seqNo
simsignal_t rcvWndSignal = cComponent::registerSignal("rcvWnd"); // rcv_wnd
simsignal_t rtoSignal = cComponent::registerSignal("rto"); // will record retransmission timeout
simsignal_t rttSignal = cComponent::registerSignal("rtt"); // will record measured RTT
simsignal_t rttvarSignal = cComponent::registerSignal("rttvar"); // will record RTT variance (rttvar)
simsignal_t sackedBytesSignal = cComponent::registerSignal("sackedBytes"); // current number of received sacked bytes
simsignal_t sndAckSignal = cComponent::registerSignal("sndAck"); // sent ackNo
simsignal_t sndSacksSignal = cComponent::registerSignal("sndSacks"); // number of sent Sacks
simsignal_t sndSeqSignal = cComponent::registerSignal("sndSeq"); // sent seqNo
simsignal_t sndWndSignal = cComponent::registerSignal("sndWnd"); // snd_wnd
simsignal_t srttSignal = cComponent::registerSignal("srtt"); // will record smoothed RTT
simsignal_t ssthreshSignal = cComponent::registerSignal("ssthresh"); // will record changes to ssthresh
simsignal_t stateSignal = cComponent::registerSignal("state"); // FSM state
simsignal_t tcpRcvPayloadBytesSignal = cComponent::registerSignal("tcpRcvPayloadBytes"); // amount of payload bytes received (including duplicates, out of order etc) for TCP throughput
simsignal_t tcpRcvQueueBytesSignal = cComponent::registerSignal("tcpRcvQueueBytes"); // current amount of used bytes in tcp receive queue
simsignal_t tcpRcvQueueDropsSignal = cComponent::registerSignal("tcpRcvQueueDrops"); // number of drops in tcp receive queue
simsignal_t unackedSignal = cComponent::registerSignal("unacked"); // number of bytes unacknowledged

} // namespace tcp
} // namespace inet

