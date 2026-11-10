// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUCLOSURESUPPORT_H
#define INET_TEST_MESHAMPDUCLOSURESUPPORT_H

class MeshAmpduClosureDriver : public cSimpleModule, public cListener
{
    bool closed = false, armed = false;
    int oldPpdus = 0, bars = 0, terminal = 0, oldSamples = 0, lateSamples = 0, expiredOriginals = 0, otherDelivered = 0, freshDelivered = 0;
    MeshAmpduHcf *hcf() const { return check_and_cast<MeshAmpduHcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    int scenario() const { return par("scenario"); }
    void source(const char *name, int destination, int length, int tid = 0) {
        auto packet = phyMsdu(name,meshMib(destination)->address,length-7);
        packet->getTagForUpdate<UserPriorityReq>()->setUserPriority(tid);
        sendDirect(packet,meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
  protected:
    void initialize() override {
        for (auto signal : {transmissionStartedSignal,transmissionEndedSignal,packetReceivedFromLowerSignal,
                packetSentToUpperSignal,packetDroppedSignal,Hcf::meshDataAttemptCompletedSignal,Hcf::meshTerminalRecoverySignal})
            getParentModule()->subscribe(signal,this);
        for (int time : {1,10,20,40,50,60,140,170,210}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("closure",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 10) meshManagement(1)->openPeer(meshMib(2)->address);
        if (step == 20) meshManagement(0)->openPeer(meshMib(3)->address);
        if (step == 40) source("other-prime",3,8);
        if (step == 50) {
            source("closure-prime",2,8);
            for (int i=0;i<(scenario() == 43 ? 3 : 6);i++) source(("closure-old"+std::to_string(i)).c_str(),2,7);
            source("unrelated-progress",3,9,1);
        }
        if (step == 1000) {
            ASSERT(!closed && hcf()->pending(meshMib(3)->address,1) == 1);
            auto radio = check_and_cast<IRadio *>(meshInterface(0)->getSubmodule("radio"));
            auto tx = check_and_cast<ITx *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("tx"));
            if (scenario() == 40) ASSERT(radio->getTransmissionState() == IRadio::TRANSMISSION_STATE_TRANSMITTING);
            if (scenario() == 41) ASSERT(hcf()->responseWaitRunning());
            if (scenario() == 42) ASSERT(tx->hasTransmission() && radio->getTransmissionState() != IRadio::TRANSMISSION_STATE_TRANSMITTING);
            closed = true;
            meshManagement(0)->closePeer(meshMib(1)->address);
            ASSERT(hcf()->numbered(meshMib(1)->address,0) == 0 && hcf()->pending(meshMib(1)->address,0) == 0);
            ASSERT(!hcf()->recovering(meshMib(1)->address,0) && hcf()->pending(meshMib(3)->address,1) == 1);
            par("releaseUnrelated").setBoolValue(true); hcf()->resumeContention();
        }
        if (step == 60 && scenario() == 43) {
            std::cout << "expired BAR recovery bars=" << bars << " pending=" << hcf()->pending(meshMib(1)->address,0)
                << " live=" << hcf()->numbered(meshMib(1)->address,0) << endl;
            ASSERT(bars == 3 && expiredOriginals == 3 && terminal == 0 && hcf()->numbered(meshMib(1)->address,0) == 0 && !hcf()->recovering(meshMib(1)->address,0));
            par("releaseUnrelated").setBoolValue(true); hcf()->resumeContention();
        }
        if (step == 140 && scenario() != 43) {
            ASSERT(closed && meshManagement(0)->getPeerState(meshMib(1)->address) == Ieee80211MgmtMesh::IDLE);
            ASSERT(meshManagement(1)->getPeerState(meshMib(0)->address) == Ieee80211MgmtMesh::IDLE);
            meshManagement(0)->openPeer(meshMib(1)->address);
        }
        if (step == 170) {
            std::cout << "fresh source at " << simTime() << " runningwait=" << hcf()->responseWaitRunning() << endl;
            std::cout << "relay peer states=" << meshManagement(1)->getPeerState(meshMib(2)->address)
                << "/" << meshManagement(2)->getPeerState(meshMib(1)->address) << endl;
            ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
            source("closure-fresh",2,11);
        }
        if (step == 210) {
            std::cout << "closure result scenario=" << scenario() << " old=" << oldPpdus << " terminal=" << terminal
                << " late=" << lateSamples << " other=" << otherDelivered << " fresh=" << freshDelivered << endl;
            ASSERT(armed || scenario() == 43);
            ASSERT(oldPpdus == 1 && terminal == 0 && lateSamples == 0 && otherDelivered == 1 && freshDelivered == 1);
            ASSERT(oldSamples == (scenario() == 40 || scenario() == 41 ? 0 : 3));
            ASSERT(!hcf()->recovering(meshMib(1)->address,0));
            emit(registerSignal("meshAmpduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        if (signal == Hcf::meshTerminalRecoverySignal && source == hcf()) terminal++;
        auto packet = dynamic_cast<const Packet *>(object);
        if (signal == transmissionStartedSignal || signal == transmissionEndedSignal)
            packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if (!packet) return;
        std::string name = packet->getName();
        if (scenario() == 43 && signal == packetDroppedSignal && source == hcf() && name.find("closure-old") != std::string::npos) {
            ASSERT(bars == 3 && hcf()->responseWaitRunning()); expiredOriginals++;
            std::cout << "covered expiry " << simTime() << " bars=" << bars << " wait=" << hcf()->responseWaitRunning() << endl;
        }
        if (signal == Hcf::meshDataAttemptCompletedSignal && source == hcf() && name.find("closure-old") != std::string::npos) {
            oldSamples++; if (closed) lateSamples++;
        }
        if (signal == packetSentToUpperSignal) {
            if (source == meshInterface(3)->getSubmodule("mac") && packet->getDataLength() == B(9)) otherDelivered++;
            if (source == meshInterface(2)->getSubmodule("mac") && packet->getDataLength() == B(11)) freshDelivered++;
        }
        if (source == meshInterface(0)->getSubmodule("radio") && (signal == transmissionStartedSignal || signal == transmissionEndedSignal)) {
            auto actual = packet->getTag<Ieee80211ModeReq>();
            if (actual->getTxVector().ampdu && name.find("closure-old") != std::string::npos) {
                if (signal == transmissionStartedSignal) { ASSERT(!closed); oldPpdus++; }
                if (!armed && ((scenario() == 40 && signal == transmissionStartedSignal) || (scenario() == 41 && signal == transmissionEndedSignal))) {
                    armed = true; scheduleAt(simTime()+SimTime(scenario() == 40 ? 1000 : 1,SIMTIME_NS),new cMessage("close-aggregate",1000));
                }
            }
            if (signal == transmissionEndedSignal && !actual->getTxVector().ampdu && !actual->getTxVector().singleMpdu) {
                auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
                if (packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())->getType() == ST_BLOCKACK_REQ) {
                    bars++; if (scenario() == 43) std::cout << "BAR completed " << bars << " at " << simTime() << endl;
                }
            }
        }
        if (scenario() == 42 && !armed && oldPpdus == 1 && source == meshInterface(0)->getSubmodule("mac")
                && signal == packetReceivedFromLowerSignal && packet->peekAtFront<Ieee80211MacHeader>()->getType() == ST_BLOCKACK
                && packet->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress() == meshMib(0)->address
                && packet->peekAtFront<Ieee80211TwoAddressHeader>()->getTransmitterAddress() == meshMib(1)->address) {
            armed = true; scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("close-prepared-successor",1000));
        }
    }
};
Define_Module(MeshAmpduClosureDriver);

#endif
