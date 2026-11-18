// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REPLY_FAILURE_TEST_PEER_H
#define __AODV_REPLY_FAILURE_TEST_PEER_H
#include "AodvPeer.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

class AodvReplyFailurePeer : public AodvPeer
{
    int failures = 0;
    int receivedAtFailedNeighbor = 0;
    int failedInterface = 0;
    int otherReplies = 0;
    bool independent = false;
    void next(double time) { step++; scheduleAt(time, timer); }
    void setRadio(bool enabled) {
        auto path = std::string("^.wlan[") + std::to_string(failedInterface) + "].radio";
        auto radio = check_and_cast<inet::physicallayer::IRadio *>(getModuleByPath(path.c_str()));
        radio->setRadioMode(enabled ? inet::physicallayer::IRadio::RADIO_MODE_TRANSCEIVER : inet::physicallayer::IRadio::RADIO_MODE_OFF);
    }
    void ask(bool cached, int output = -1) {
        output = cached ? 0 : (output < 0 ? failedInterface : output);
        auto request = makeShared<Rreq>();
        request->setPacketType(RREQ);
        request->setChunkLength(B(24));
        request->setOriginatorAddr(Ipv4Address(output == 0 ? "10.0.0.2" : "10.0.0.3"));
        request->setOriginatorSeqNum(1);
        request->setRreqId(step + 1);
        request->setDestAddr(cached ? destination : dut);
        request->setUnknownSeqNumFlag(true);
        request->setGratuitousRREPFlag(cached);
        transmit(request, output);
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage == INITSTAGE_APPLICATION_LAYER) {
            failedInterface = par("scenario").stdstringValue() == "FailedReplyNoAck" ? 1 : 0;
            independent = par("scenario").stdstringValue() == "FailedReplyIndependent";
            getModuleByPath("^.^.dut")->subscribe(linkBrokenSignal, this);
        }
    }
    void handleMessage(cMessage *message) override {
        if (message != timer) { AodvPeer::handleMessage(message); return; }
        if (failedInterface == 1) {
            if (step == 0) {
                auto reply = makeShared<Rrep>();
                reply->setPacketType(RREP);
                reply->setChunkLength(B(20));
                reply->setDestAddr(destination);
                reply->setOriginatorAddr(dut);
                reply->setDestSeqNum(20);
                reply->setLifeTime(5);
                transmit(reply, 1);
                next(0.095);
                return;
            }
            if (step == 1) { setRadio(false); next(0.1); return; }
            if (step == 2) { ask(true); next(0.13); return; }
        }
        else {
            if (independent && simTime() == SimTime(0.095)) { ask(false, 1); next(0.1); return; }
            if (simTime() == SimTime(0.1)) { ask(false); next(0.1007); return; }
            if (simTime() == SimTime(0.1007)) { setRadio(false); next(0.13); return; }
        }
        if (simTime() == SimTime(0.13)) {
            ASSERT(failures == 1);
            setRadio(true);
            auto ack = makeShared<RrepAck>();
            ack->setPacketType(RREPACK);
            ack->setChunkLength(B(2));
            transmit(ack, failedInterface);
            next(0.14);
            return;
        }
        if (simTime() == SimTime(0.14)) { ask(false); next(0.16); return; }
        if (independent && simTime() == SimTime(0.16)) { ask(false, 1); next(0.18); return; }
        ASSERT(failures == 1 && receivedAtFailedNeighbor == 0);
        if (independent)
            ASSERT(otherReplies == 1);
        pass(par("scenario").stdstringValue());
    }
    void socketDataArrived(UdpSocket *socket, Packet *packet) override {
        if (dynamicPtrCast<const Rrep>(packet->peekAtFront<AodvControlPacket>())) {
            auto target = Ipv4Address(failedInterface == 0 ? "10.0.0.2" : "10.0.0.3");
            if (packet->getTag<L3AddressInd>()->getDestAddress() == target)
                receivedAtFailedNeighbor++;
            if (independent && packet->getTag<L3AddressInd>()->getDestAddress() == Ipv4Address("10.0.0.3")) {
                otherReplies++;
                delete packet;
                return;
            }
        }
        AodvPeer::socketDataArrived(socket, packet);
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method("observeFailedReply");
        auto packet = dynamic_cast<Packet *>(object);
        if (signal != linkBrokenSignal || !packet)
            return;
        auto ip = getNetworkProtocolHeader(packet);
        auto udp = packet->peekDataAt<UdpHeader>(ip->getChunkLength());
        ASSERT(udp->getDestPort() == controlPort);
        auto reply = packet->peekDataAt<Rrep>(ip->getChunkLength() + udp->getChunkLength());
        ASSERT(reply->getAckRequiredFlag() == (failedInterface == 0));
        auto interfaces = check_and_cast<IInterfaceTable *>(getModuleByPath("^.interfaceTable"));
        auto failed = interfaces->findInterfaceByName(failedInterface == 0 ? "wlan0" : "wlan1");
        ASSERT(packet->getTag<MacAddressReq>()->getDestAddress() == failed->getMacAddress());
        ASSERT(packet->findTag<InterfaceReq>());
        failures++;
    }
};
Define_Module(AodvReplyFailurePeer);
#endif
