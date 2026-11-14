// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUWRAPSUPPORT_H
#define INET_TEST_MESHAMPDUWRAPSUPPORT_H

class MeshAmpduWrapDriver : public cSimpleModule, public cListener
{
    int delivered = 0, failed = 0, retries = 0, attempts = 0;
    bool saw4095 = false, sawWrappedZero = false;
    double expectedError = 0;
    std::map<uint32_t,int> identities;
  protected:
    void initialize() override {
        for (auto signal : {transmissionEndedSignal,packetSentToUpperSignal,Hcf::meshDataAttemptCompletedSignal,Hcf::meshTerminalRecoverySignal})
            getParentModule()->subscribe(signal,this);
        for (int time : {1,40,50,210}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("wrap",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 40)
            sendDirect(phyMsdu("wrap-prime",meshMib(1)->address,1),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
        if (step == 50) {
            ASSERT(check_and_cast<IMeshPathSelection *>(meshInterface(0)->getSubmodule("pathSelection"))->findPath(meshMib(1)->address));
            for (int i=0;i<4097;i++) sendDirect(phyMsdu(("wrap-data"+std::to_string(i)).c_str(),meshMib(1)->address),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
        }
        if (step == 210) {
            std::cout << "wrap result delivered=" << delivered << " attempts=" << attempts << " retry=" << retries << " failed=" << failed << endl;
            ASSERT(delivered == 4097 && identities.size() == 4097 && saw4095 && sawWrappedZero && failed == 1 && retries == 1);
            ASSERT(attempts == 4098);
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address) == expectedError);
            emit(registerSignal("meshAmpduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        if (signal == Hcf::meshTerminalRecoverySignal) ASSERT(false);
        auto packet = dynamic_cast<const Packet *>(object);
        if (signal == transmissionEndedSignal) packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if (!packet || std::string(packet->getName()).find("wrap-data") == std::string::npos) return;
        if (signal == packetSentToUpperSignal && source == meshInterface(1)->getSubmodule("mac")) delivered++;
        if (signal == Hcf::meshDataAttemptCompletedSignal && source == meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")) {
            auto attempt = check_and_cast<MeshDataAttempt *>(details);
            ASSERT(!attempt->terminalTimeout); attempts++; failed += !attempt->acknowledged;
            expectedError = expectedError*0.875+(!attempt->acknowledged ? 0.125 : 0);
        }
        if (signal != transmissionEndedSignal || source != meshInterface(0)->getSubmodule("radio")) return;
        auto actual = packet->getTag<Ieee80211ModeReq>();
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        Ieee80211AmpduLayout layout;
        if (!actual->getTxVector().ampdu) return;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),actual->getTxVector().format == Ieee80211TxVector::VHT,layout));
        for (const auto& item : layout.members) {
            Packet member("wrapped-member",packet->peekDataAt(phy->getChunkLength()+item.offset,item.length));
            auto header = member.peekAtFront<Ieee80211MeshDataHeader>();
            auto control = member.peekDataAt<Ieee80211MeshControl>(header->getChunkLength(),B(6));
            auto old = identities.find(control->getSequenceNumber());
            if (old == identities.end()) identities.emplace(control->getSequenceNumber(),header->getSequenceNumber().get());
            else { ASSERT(header->getRetry() && old->second == header->getSequenceNumber().get()); retries++; }
            if (header->getSequenceNumber().get() == 4095) saw4095 = true;
            if (saw4095 && header->getSequenceNumber().get() == 0) sawWrappedZero = true;
        }
    }
};
Define_Module(MeshAmpduWrapDriver);

#endif
