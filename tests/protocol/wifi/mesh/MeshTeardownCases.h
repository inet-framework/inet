// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_TEST_MESHTEARDOWNCASES_H
#define __INET_TEST_MESHTEARDOWNCASES_H

class MeshTeardownDriver : public cSimpleModule, public cListener
{
    int phase = -1;
    int oldTransmissions[3] = {};
    bool closed[3] = {};
    int freshDelivered = 0;
    int otherDelivered = 0;
    const FrameSequenceContext *active = nullptr;
    void source(const char *name, int destination, int tid) {
        sendDirect(fixtureMsdu(name,meshMib(destination)->address,tid),meshInterface(1)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    ITx *transmitter() { return check_and_cast<ITx *>(meshInterface(1)->getSubmodule("mac")->getSubmodule("tx")); }
    int queuedCategories(MacAddress peer) {
        int categories = 0;
        auto edca = meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf")->getSubmodule("edca");
        for (int ac = 0; ac < 4; ac++) {
            auto edcaf = edca->getSubmodule("edcaf",ac);
            auto pending = check_and_cast<queueing::IPacketQueue *>(edcaf->getSubmodule("pendingQueue"));
            auto frames = check_and_cast<InProgressFrames *>(edcaf->getSubmodule("inProgressFrames"));
            bool found = false;
            for (int i = 0; i < pending->getNumPackets(); i++) {
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(pending->getPacket(i)->peekAtFront<Ieee80211MacHeader>());
                found |= data && data->getReceiverAddress() == peer;
            }
            for (int i = 0; i < frames->getLength(); i++) {
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>());
                found |= data && data->getReceiverAddress() == peer;
            }
            categories += found;
        }
        return categories;
    }
    void checkRetired() {
        auto edca = meshInterface(1)->getSubmodule("mac")->getSubmodule("hcf")->getSubmodule("edca");
        for (int ac = 0; ac < 4; ac++) {
            auto edcaf = edca->getSubmodule("edcaf",ac);
            auto pending = check_and_cast<queueing::IPacketQueue *>(edcaf->getSubmodule("pendingQueue"));
            auto frames = check_and_cast<InProgressFrames *>(edcaf->getSubmodule("inProgressFrames"));
            for (int i = 0; i < pending->getNumPackets(); i++) {
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(pending->getPacket(i)->peekAtFront<Ieee80211MacHeader>());
                ASSERT(!data || data->getReceiverAddress() != meshMib(0)->address);
            }
            for (int i = 0; i < frames->getLength(); i++) {
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>());
                ASSERT(!data || data->getReceiverAddress() != meshMib(0)->address);
            }
        }
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto name : {"packetSentToLower","packetReceivedFromLower","packetSentToUpper","packetDropped","packetEnqueued","frameSequenceStarted","frameSequenceFinished"})
            network->subscribe(registerSignal(name),this);
        for (int p = 0; p < 3; p++) {
            scheduleAt(SimTime(200+25*p,SIMTIME_MS),new cMessage("begin",100+p));
            scheduleAt(SimTime(208+25*p,SIMTIME_MS),new cMessage("reopen",200+p));
            scheduleAt(SimTime(215+25*p,SIMTIME_MS),new cMessage("fresh",300+p));
        }
        scheduleAt(SimTime(280,SIMTIME_MS),new cMessage("verify",400));
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        auto mac = meshInterface(1)->getSubmodule("mac");
        if (source == mac->getSubmodule("hcf")) {
            if (signal == registerSignal("frameSequenceStarted")) active = check_and_cast<FrameSequenceContext *>(object);
            if (signal == registerSignal("frameSequenceFinished")) active = nullptr;
        }
        auto packet = dynamic_cast<Packet *>(object); if (!packet || phase < 0) return;
        if ((signal == registerSignal("packetDropped") || signal == registerSignal("packetEnqueued"))
                && std::string(packet->getName()) == "old-teardown")
            ASSERT(packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ieee80211Mac);
        if (signal == registerSignal("packetSentToUpper")) {
            if (source == meshInterface(0)->getSubmodule("mac") && std::string(packet->getName()) == "fresh-teardown") freshDelivered++;
            if (source == meshInterface(2)->getSubmodule("mac") && std::string(packet->getName()) == "other-peer") otherDelivered++;
        }
        if (source != mac) return;
        if (signal == registerSignal("packetSentToLower") && std::string(packet->getName()) == "old-teardown") {
            oldTransmissions[phase]++;
            ASSERT(!closed[phase]);
            if (oldTransmissions[phase] == 1 && phase != 0) {
                auto delay = phase == 1 ? SimTime(1,SIMTIME_US)
                    : Ieee80211OfdmCompliantModes::ofdmMode6MbpsCS20MHz.getDuration(packet->getDataLength()) + SimTime(1,SIMTIME_US);
                scheduleAt(simTime()+delay,new cMessage("close",500+phase));
            }
        }
        if (signal == registerSignal("packetReceivedFromLower") && phase == 0 && !closed[0]
                && oldTransmissions[0] == 1 && packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_ACK)
            scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("close-ifs",500));
    }
    void handleMessage(cMessage *message) override {
        int kind = message->getKind();
        if (kind >= 500) {
            auto radio = check_and_cast<physicallayer::IRadio *>(meshInterface(1)->getSubmodule("radio"));
            ASSERT(active);
            if (phase == 0) {
                ASSERT(transmitter()->hasTransmission() && radio->getTransmissionState() != physicallayer::IRadio::TRANSMISSION_STATE_TRANSMITTING);
                ASSERT(dynamic_cast<ITransmitStep *>(active->getLastStep()));
            }
            if (phase == 1) ASSERT(transmitter()->hasTransmission() && radio->getTransmissionState() == physicallayer::IRadio::TRANSMISSION_STATE_TRANSMITTING);
            if (phase == 2) ASSERT(!transmitter()->hasTransmission() && dynamic_cast<IReceiveStep *>(active->getLastStep()));
            ASSERT(queuedCategories(meshMib(0)->address) >= 2);
            ASSERT(queuedCategories(meshMib(2)->address) == 1);
            closed[phase] = true;
            meshManagement(1)->closePeer(meshMib(0)->address);
            checkRetired();
            ASSERT(queuedCategories(meshMib(2)->address) == 1);
            if (phase != 1) ASSERT(!active);
        }
        else if (kind >= 400) {
            ASSERT(freshDelivered == 3 && otherDelivered == 3);
            for (int p = 0; p < 3; p++) ASSERT(closed[p] && oldTransmissions[p] == 1);
            emit(registerSignal("meshTeardownComplete"),1L);
        }
        else if (kind >= 300) {
            ASSERT(meshMib(1)->isMeshPeerEstablished(meshMib(0)->address));
            source("fresh-teardown",0,7);
        }
        else if (kind >= 200) {
            ASSERT(meshManagement(1)->getPeerState(meshMib(0)->address) == Ieee80211MgmtMesh::IDLE);
            ASSERT(meshManagement(0)->getPeerState(meshMib(1)->address) == Ieee80211MgmtMesh::IDLE);
            meshManagement(1)->openPeer(meshMib(0)->address);
        }
        else {
            phase = kind-100;
            auto expiry = SimTime(600,SIMTIME_MS);
            fixturePath(1,0,0,expiry); fixturePath(1,2,2,expiry);
            fixturePrecursor(0,0,1,expiry); fixturePrecursor(2,2,1,expiry);
            ASSERT(meshMib(1)->isMeshPeerEstablished(meshMib(0)->address));
            source("old-teardown",0,7); source("old-teardown",0,7); source("old-teardown",0,0);
            source("other-peer",2,1);
        }
        delete message;
    }
};
Define_Module(MeshTeardownDriver);
#endif
