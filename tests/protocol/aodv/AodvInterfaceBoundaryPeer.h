// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_INTERFACE_BOUNDARY_PEER_H
#define __AODV_INTERFACE_BOUNDARY_PEER_H
#include "AodvPeer.h"

class AodvInterfaceBoundaryPeer : public AodvPeer
{
    std::string mode;
    cMessage *change = nullptr;
    int oldId = -1, secondaryOutputs = 0, data = 0;
    bool changed = false, armed = false;
    std::set<int> macIds;
    IInterfaceTable *interfaces() { return check_and_cast<IInterfaceTable *>(getModuleByPath("^.^.dut.interfaceTable")); }
    void offer(uint32_t sequence, int link, simtime_t lifetime) {
        auto reply = makeShared<Rrep>(); reply->setPacketType(RREP); reply->setChunkLength(B(20));
        reply->setDestAddr(destination); reply->setDestSeqNum(sequence); reply->setOriginatorAddr(dut);
        reply->setHopCount(1); reply->setLifeTime(lifetime);
        transmit(reply, link, link ? L3Address(Ipv4Address("10.1.0.1")) : dut);
    }
    void removeSecondary() {
        auto secondary = interfaces()->findInterfaceByName("wlan1");
        oldId = secondary->getInterfaceId(); changed = true;
        interfaces()->deleteInterface(secondary);
    }
    void run() {
        if (mode == "HistoryExpiry") {
            if (step == 0) offer(20, 1, 0.1);
            if (step == 1) removeSecondary();
            if (step == 2) offer(20, 0, 1);
            if (step == 3) { auto route = findRoute(destination); ASSERT(!route || !check_and_cast<AodvRouteData *>(route->getProtocolData())->isActive()); }
            if (step == 4) offer(1, 0, 1);
            if (step == 5) {
                auto route = findRoute(destination); ASSERT(route && check_and_cast<AodvRouteData *>(route->getProtocolData())->getDestSeqNum() == 1);
                ASSERT(data == 1 && AodvDataCounters::created == 1 && AodvDataCounters::destroyed == 1);
                std::cout << "AODV InterfaceBoundary HistoryExpiry: PASS\n"; return;
            }
            static const double times[] = {0.1, 0.15, 0.3, 0.4, 0.7, 0.9};
            ++step; scheduleAt(times[step], timer);
        }
        else {
            if (step == 0) {
                auto request = makeShared<Rreq>(); request->setPacketType(RREQ); request->setChunkLength(B(24));
                request->setOriginatorAddr(peer); request->setOriginatorSeqNum(10); request->setRreqId(70);
                request->setDestAddr(Ipv4Address("198.51.100.77")); request->setUnknownSeqNumFlag(true);
                transmit(request); ++step; scheduleAt(0.6, timer);
            }
            else {
                ASSERT(armed && changed);
                ASSERT(secondaryOutputs == (mode == "SendFirst" ? 1 : 0));
                std::cout << "AODV InterfaceBoundary " << mode << ": PASS\n";
            }
        }
    }
  protected:
    void initialize(int stage) override {
        AodvPeer::initialize(stage);
        if (stage != INITSTAGE_APPLICATION_LAYER) return;
        mode = par("scenario").stdstringValue();
        oldId = interfaces()->findInterfaceByName("wlan1")->getInterfaceId();
        dutIp = getModuleByPath("^.^.dut.ipv4.ip");
        dutIp->subscribe(packetReceivedFromLowerSignal, this);
        for (int i = 0; i < 2; ++i) {
            auto mac = interfaces()->findInterfaceByName(("wlan" + std::to_string(i)).c_str())->getSubmodule("mac");
            macIds.insert(mac->getId()); mac->subscribe(packetReceivedFromUpperSignal, this);
        }
        change = new cMessage("same-time-interface-delete");
        change->setSchedulingPriority(mode == "SendFirst" ? 1 : -1);
    }
    void handleMessage(cMessage *message) override {
        if (message == change) removeSecondary();
        else if (message == timer) run();
        else socket.processMessage(message);
    }
    void socketDataArrived(UdpSocket *, Packet *packet) override { delete packet; }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *value, cObject *) override {
        Enter_Method("observeInterfaceBoundary");
        auto packet = dynamic_cast<Packet *>(value); if (!packet) return;
        auto header = packet->peekAtFront<Ipv4Header>();
        bool output = macIds.count(source->getId());
        if (source != dutIp && !output) { if (!strncmp(packet->getName(), "queued-data-", 12)) ++data; return; }
        if (header->getProtocolId() != IP_PROT_UDP) return;
        auto udp = packet->peekDataAt<UdpHeader>(header->getChunkLength());
        if (udp->getDestPort() != 654) return;
        auto control = packet->peekDataAt<AodvControlPacket>(header->getChunkLength() + udp->getChunkLength());
        if (!dynamicPtrCast<const Rreq>(control)) return;
        if (signal == packetReceivedFromLowerSignal && mode != "HistoryExpiry" && !armed) {
            armed = true; scheduleAt(simTime() + SimTime(0.2), change);
        }
        if (output && packet->getTag<InterfaceReq>()->getInterfaceId() == oldId) {
            ASSERT(!changed); ++secondaryOutputs;
        }
    }
  public:
    ~AodvInterfaceBoundaryPeer() { cancelAndDelete(change); }
};
Define_Module(AodvInterfaceBoundaryPeer);
#endif
