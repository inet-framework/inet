// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_ACK_DEADLINE_TEST_PEER_H
#define __AODV_ACK_DEADLINE_TEST_PEER_H
#include "AodvPeer.h"

class AodvAckDeadlinePeer : public AodvPeer
{
    simtime_t requestLatency;
    simtime_t ackLatency;
    simtime_t mainReception;
    simtime_t ackReception;
    simtime_t ackSend;
    int replyCount = 0;
    void sendControl(const Ptr<AodvControlPacket>& control, const char *name) {
        auto packet = new Packet(name, control);
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        auto selected = interfaces->findInterfaceByName("eth0");
        packet->addTag<InterfaceReq>()->setInterfaceId(selected->getInterfaceId());
        packet->addTag<L3AddressReq>()->setSrcAddress(peer);
        packet->addTag<NextHopAddressReq>()->setNextHopAddress(dut);
        packet->addTag<HopLimitReq>()->setHopLimit(1);
        socket.sendTo(packet, dut, 654);
    }
    void sendRequest(uint32_t id, bool local, const char *name) {
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(peer);
        request->setOriginatorSeqNum(id);
        request->setRreqId(id);
        request->setDestAddr(local ? dut : destination);
        request->setUnknownSeqNumFlag(true);
        sendControl(request, name);
    }
    void sendAck(const char *name) {
        auto ack = makeShared<RrepAck>();
        ack->setPacketType(RREPACK);
        ack->setChunkLength(B(2));
        sendControl(ack, name);
    }
    void next(simtime_t time) { step++; scheduleAt(time, timer); }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            dutIp = getModuleByPath("^.^.dut.ipv4.ip");
            dutIp->subscribe(packetReceivedFromLowerSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        if (step == 0) { sendRequest(1, false, "calibrate-request"); next(0.04); return; }
        if (step == 1) { sendAck("calibrate-ack"); next(0.15); return; }
        if (step == 2) {
            ASSERT(requestLatency > SIMTIME_ZERO && ackLatency > SIMTIME_ZERO);
            // A channel after real UDP reception controls when the final socketIn
            // event enters the FES, before/after the request creates its ACK timer.
            simtime_t applicationDelay = getModuleByPath("^.^.dut")->par("applicationDelay");
            ASSERT((applicationDelay > SimTime(0.05)) == par("ackFirst").boolValue());
            ackSend = SimTime(0.2) + requestLatency + SimTime(0.05) - ackLatency;
            next(0.2);
            return;
        }
        if (step == 3) { sendRequest(2, true, "main-request"); next(ackSend); return; }
        if (step == 4) { sendAck("main-ack"); next(0.6); return; }
        if (step == 5) { sendRequest(3, true, "probe-request"); next(0.9); return; }
        ASSERT(ackReception == mainReception + SimTime(0.05));
        bool first = par("ackFirst");
        ASSERT(replyCount == (first ? 2 : 1));
        checkSequence(peer, first ? 3 : 2, true);
        pass(first ? "AckFirstAtDeadline" : "TimeoutFirstAtDeadline");
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override {
        if (dynamicPtrCast<const Rrep>(packet->peekAtFront<AodvControlPacket>()))
            replyCount++;
        delete packet; // Only the explicitly timed ACK is sent.
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeControlDeadline");
        if (source != dutIp || signal != packetReceivedFromLowerSignal)
            return;
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet)
            return;
        std::string name = packet->getName();
        if (name == "calibrate-request") requestLatency = simTime() - SimTime(0.01);
        else if (name == "calibrate-ack") ackLatency = simTime() - SimTime(0.04);
        else if (name == "main-request") mainReception = simTime();
        else if (name == "main-ack") ackReception = simTime();
    }
};
Define_Module(AodvAckDeadlinePeer);
#endif
