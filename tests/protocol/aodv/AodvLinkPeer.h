// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_LINK_TEST_PEER_H
#define __AODV_LINK_TEST_PEER_H
#include "AodvPeer.h"

class AodvLinkPeer : public AodvPeer
{
    int acknowledgments = 0;
    std::map<L3Address, int> repliesTo;
    void next(double time) { step++; scheduleAt(time, timer); }
    void acknowledge(int output = 0) {
        auto ack = makeShared<RrepAck>();
        ack->setPacketType(RREPACK);
        ack->setChunkLength(B(2));
        transmit(ack, output);
    }
    void ask(int output = 0, bool forSelf = true) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(Ipv4Address(output ? "10.0.0.3" : "10.0.0.2"));
        request->setOriginatorSeqNum(1);
        request->setRreqId(step + 1);
        request->setDestAddr(forSelf ? dut : destination);
        request->setUnknownSeqNumFlag(true);
        transmit(request, output);
    }
    void offer(uint32_t sequence, simtime_t lifetime, bool toOriginator, bool ack = true) {
        auto reply = makeShared<Rrep>();
        reply->setPacketType(RREP);
        reply->setChunkLength(B(20));
        reply->setDestAddr(destination);
        reply->setOriginatorAddr(toOriginator ? dut : peer);
        reply->setDestSeqNum(sequence);
        reply->setLifeTime(lifetime);
        reply->setAckRequiredFlag(ack);
        transmit(reply);
    }
  protected:
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        auto scenario = par("scenario").stdstringValue();
        if (scenario == "OriginatorAck") {
            if (step == 0) { offer(20, 5, true); next(0.15); return; }
            if (step == 1) { ASSERT(acknowledgments == 1); offer(19, 5, true); next(0.2); return; }
            if (step == 2) { ASSERT(acknowledgments == 2); offer(21, 0, true); next(0.25); return; }
            ASSERT(acknowledgments == 3);
            checkSequence(destination, 20, true);
        }
        else if (scenario == "DirectInactiveAck") {
            if (step == 0) { destination = peer; offer(20, 5, true, false); next(0.12); return; }
            if (step == 1) { sendError(21); next(0.14); return; }
            if (step == 2) { offer(20, 5, true); next(0.16); return; }
            ASSERT(acknowledgments == 1);
            checkSequence(peer, 21, false);
        }
        else if (scenario == "RejectedAck") {
            if (step == 0) { ask(0, false); next(0.15); return; }
            if (step == 1) { offer(20, 5, false, false); next(0.2); return; }
            if (step == 2) { offer(19, 5, false); next(0.25); return; }
            ASSERT(acknowledgments == 1 && repliesTo[peer] == 1);
            checkSequence(destination, 20, true);
        }
        else if (scenario == "IndependentAck") {
            if (step == 0) { ask(); next(0.11); return; }
            if (step == 1) { ask(1); next(0.12); return; }
            if (step == 2) { acknowledge(); next(0.18); return; }
            if (step == 3) { ask(1); next(0.19); return; }
            if (step == 4) { ask(); next(0.2); return; }
            if (step == 5) { acknowledge(1); next(0.21); return; }
            if (step == 6) { ask(1); next(0.22); return; }
            ASSERT(repliesTo[peer] == 2 && repliesTo[Ipv4Address("10.0.0.3")] == 1);
        }
        else if (scenario == "SharedAckDeadline") {
            if (step == 0) { ask(); next(0.13); return; }
            if (step == 1) { ask(); next(0.16); return; }
            if (step == 2) { ask(); next(0.22); return; }
            ASSERT(repliesTo[peer] == 2);
        }
        else if (scenario == "UnsolicitedAck") {
            if (step == 0) { destination = peer; offer(20, 5, true, false); next(0.12); return; }
            if (step == 1) { sendError(21); next(0.13); return; }
            if (step == 2) { ask(1); next(0.14); return; }
            if (step == 3) { acknowledge(); next(0.16); return; }
            if (step == 4) { checkSequence(peer, 21, false); next(0.21); return; }
            if (step == 5) { ask(1); next(0.23); return; }
            ASSERT(repliesTo[Ipv4Address("10.0.0.3")] == 1);
            checkSequence(peer, 21, false);
        }
        else
            throw cRuntimeError("Unknown link scenario");
        pass(scenario);
    }
    void socketDataArrived(UdpSocket *socket, Packet *packet) override {
        auto control = packet->peekAtFront<AodvControlPacket>();
        if (dynamicPtrCast<const RrepAck>(control)) {
            acknowledgments++;
            ASSERT(packet->getTag<HopLimitInd>()->getHopLimit() == 1);
        }
        else if (dynamicPtrCast<const Rrep>(control))
            repliesTo[packet->getTag<L3AddressInd>()->getDestAddress()]++;
        delete packet; // ACK scripts deliberately control each link independently
    }
};
Define_Module(AodvLinkPeer);
#endif
