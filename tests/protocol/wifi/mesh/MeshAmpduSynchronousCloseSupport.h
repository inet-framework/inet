// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUSYNCHRONOUSCLOSESUPPORT_H
#define INET_TEST_MESHAMPDUSYNCHRONOUSCLOSESUPPORT_H

class MeshAmpduSynchronousCloseDriver : public cSimpleModule, public cListener
{
    bool closed = false;
    int samples = 0, terminals = 0, terminalDrops = 0, oldPpdus = 0, otherDelivered = 0, freshDelivered = 0;
    MeshAmpduHcf *hcf() const { return check_and_cast<MeshAmpduHcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")); }
    int scenario() const { return par("scenario"); }
    void source(const char *name, int destination, int size, int tid = 0) {
        auto packet = phyMsdu(name,meshMib(destination)->address,size-7);
        packet->getTagForUpdate<UserPriorityReq>()->setUserPriority(tid);
        sendDirect(packet,meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void closeSynchronously() {
        ASSERT(!closed && hcf()->pending(meshMib(3)->address,1) == 1);
        closed = true;
        // Deliberately call from the current notification stack, not a later
        // event. Its frame-sequence handler still owns the resolving step.
        meshManagement(0)->closePeer(meshMib(1)->address);
        ASSERT(hcf()->numbered(meshMib(1)->address,0) == 0 && !hcf()->recovering(meshMib(1)->address,0));
        ASSERT(hcf()->pending(meshMib(3)->address,1) == 1);
        par("releaseUnrelated").setBoolValue(true);
        auto metric = check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"));
        ASSERT(metric->getErrorEstimate(meshMib(1)->address) == 0 && metric->getPayloadEstimate(meshMib(1)->address) == 1);
    }
  protected:
    void initialize() override {
        for (auto signal : {packetDroppedSignal,packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal,
                Hcf::meshTerminalRecoverySignal,transmissionStartedSignal}) getParentModule()->subscribe(signal,this);
        for (int time : {1,10,20,40,50,140,170,210}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("synchronous-close",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 10) meshManagement(1)->openPeer(meshMib(2)->address);
        if (step == 20) meshManagement(0)->openPeer(meshMib(3)->address);
        if (step == 40) source("sync-other-prime",3,8);
        if (step == 50) {
            source("sync-prime",2,8);
            for (int i=0;i<3;i++) source(("sync-old"+std::to_string(i)).c_str(),2,7);
            source("unrelated-progress",3,9,1);
        }
        if (step == 140) {
            ASSERT(closed && meshManagement(0)->getPeerState(meshMib(1)->address) == Ieee80211MgmtMesh::IDLE);
            ASSERT(meshManagement(1)->getPeerState(meshMib(0)->address) == Ieee80211MgmtMesh::IDLE);
            meshManagement(0)->openPeer(meshMib(1)->address);
        }
        if (step == 170) source("sync-fresh",2,11);
        if (step == 210) {
            ASSERT(closed && otherDelivered == 1 && freshDelivered == 1);
            ASSERT(samples == (scenario() == 47 ? 9 : scenario() == 48 ? 3 : 1));
            ASSERT(terminals == (scenario() == 47 ? 1 : 0) && terminalDrops == (scenario() == 47 ? 3 : scenario() == 48 ? 1 : 0));
            ASSERT(oldPpdus == (scenario() == 47 ? 3 : 1));
            std::cout << "synchronous close scenario=" << scenario() << " samples=" << samples << " terminal=" << terminals << " drops=" << terminalDrops << endl;
            emit(registerSignal("meshAmpduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        auto packet = dynamic_cast<const Packet *>(object);
        if (signal == transmissionStartedSignal) packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if (!packet) return;
        std::string name = packet->getName();
        if (signal == packetSentToUpperSignal) {
            if (source == meshInterface(3)->getSubmodule("mac") && packet->getDataLength() == B(9)) otherDelivered++;
            if (source == meshInterface(2)->getSubmodule("mac") && packet->getDataLength() == B(11)) freshDelivered++;
        }
        if (source == hcf() && signal == Hcf::meshTerminalRecoverySignal) {
            ASSERT(!closed && scenario() == 47); terminals++;
            auto fact = check_and_cast<MeshTerminalRecovery *>(details);
            ASSERT(fact->retiredMembers == 3 && fact->reason == MeshTerminalRecovery::DATA_RETRY_LIMIT);
            ASSERT(hcf()->numbered(meshMib(1)->address,0) == 0);
            closeSynchronously(); return;
        }
        if (name.find("sync-old") == std::string::npos) return;
        if (signal == transmissionStartedSignal && source == meshInterface(0)->getSubmodule("radio")) { ASSERT(!closed); oldPpdus++; }
        if (source != hcf()) return;
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            ASSERT(!closed); samples++;
            auto attempt = check_and_cast<MeshDataAttempt *>(details);
            ASSERT(!attempt->terminalTimeout);
            if (scenario() == 45 || scenario() == 46) {
                ASSERT(samples == 1 && attempt->acknowledged == (scenario() == 45)); closeSynchronously();
            }
        }
        if (signal == packetDroppedSignal) {
            auto drop = dynamic_cast<PacketDropDetails *>(details);
            if (drop && drop->getReason() == RETRY_LIMIT_REACHED) {
                ASSERT(!closed); terminalDrops++;
                if (scenario() == 48) { ASSERT(terminalDrops == 1); closeSynchronously(); }
            }
        }
    }
};
Define_Module(MeshAmpduSynchronousCloseDriver);

#endif
