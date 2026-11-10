// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUSEMANTICSUPPORT_H
#define INET_TEST_MESHAMPDUSEMANTICSUPPORT_H

// Receiver-only reconstruction: preserve the actual PHY vector and symbol
// allocation, but replace native bodies with independently checksummed bytes.
struct MeshSemanticFault
{
    std::vector<std::vector<uint8_t>> originalBodies;
    int receptions = 0;
    void apply(Packet *packet, int scenario) {
        auto actual = packet->findTag<Ieee80211ModeInd>();
        if (!actual || !actual->getTxVector().ampdu
                || std::string(packet->getName()).find("semantic-") == std::string::npos) return;
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        bool vht = actual->getTxVector().format == Ieee80211TxVector::VHT;
        Ieee80211AmpduLayout layout;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),vht,layout));
        ASSERT(layout.members.size() == 3 && receptions < 2);
        auto psdu = packet->peekDataAt<BytesChunk>(phy->getChunkLength(),phy->getLengthField())->getBytes();
        auto encodeDelimiter = [vht](int length) {
            auto delimiter = makeShared<Ieee80211MpduSubframeHeader>();
            delimiter->setVht(vht); delimiter->setEof(false); delimiter->setLength(length);
            Packet encoded("delimiter",delimiter);
            return encoded.peekDataAsBytes()->getBytes();
        };
        auto spacing = encodeDelimiter(0);
        for (size_t i=0;i<layout.members.size();i++) {
            const auto& item = layout.members[i];
            auto bytes = packet->peekDataAt<BytesChunk>(phy->getChunkLength()+item.offset,item.length)->getBytes();
            Packet decoded("header",makeShared<BytesChunk>(bytes));
            auto header = decoded.peekAtFront<Ieee80211MeshDataHeader>();
            size_t headerLength = header->getChunkLength().get<B>();
            std::vector<uint8_t> body(bytes.begin()+headerLength,bytes.end()-4);
            if (receptions == 0) {
                originalBodies.push_back(body);
                // The last member keeps the aggregate's physical extent. Its
                // TTL error is also MAC-correct and must get the same BA bit.
                if (scenario == 20 || i == 2) body[1] = 0;
                if (scenario == 30) bytes[headerLength-1] &= ~1; // Same high QoS octet in every member.
                if (i != 2) {
                    if (scenario == 21) body[0] = 1;
                    if (scenario >= 22 && scenario <= 27) body.resize(scenario-22);
                    if (scenario == 28) body.push_back(0xab);
                    if (scenario == 29) bytes[headerLength-2] |= 0x80;
                }
            }
            else {
                // Fresh hop MAC sequence, same original mesh identity: none of
                // the rejected bodies may have populated mesh duplicate state.
                body = originalBodies.at(i);
            }
            bytes.resize(headerLength);
            bytes.insert(bytes.end(),body.begin(),body.end());
            uint32_t fcs = independentMeshFcs(bytes,0,bytes.size());
            for (int shift=0;shift<32;shift+=8) bytes.push_back(uint8_t(fcs>>shift));
            size_t cursor = item.delimiterOffset.get<B>();
            auto delimiter = encodeDelimiter(bytes.size());
            std::copy(delimiter.begin(),delimiter.end(),psdu.begin()+cursor); cursor += 4;
            std::copy(bytes.begin(),bytes.end(),psdu.begin()+cursor); cursor += bytes.size();
            if (i+1 < layout.members.size()) {
                size_t next = layout.members[i+1].delimiterOffset.get<B>();
                while (cursor%4) psdu.at(cursor++) = 0;
                while (cursor < next) {
                    ASSERT(cursor+4 <= next);
                    std::copy(spacing.begin(),spacing.end(),psdu.begin()+cursor); cursor += 4;
                }
                ASSERT(cursor == next);
            }
        }
        packet->replaceDataAt(makeShared<BytesChunk>(psdu),phy->getChunkLength(),phy->getLengthField());
        Ieee80211AmpduLayout checked;
        ASSERT(findAmpdu(packet,phy->getChunkLength(),phy->getLengthField(),vht,checked));
        for (const auto& item : checked.members)
            ASSERT(isAmpduMemberFcsOk(packet,phy->getChunkLength()+item.offset,item.length));
        receptions++;
    }
};

class MeshAmpduSemanticDriver : public cSimpleModule, public cListener
{
    int delivered = 0, samples = 0, failures = 0, droppedBodies = 0, sourcePpdus = 0;
    void source(const char *name, int length) {
        sendDirect(phyMsdu(name,meshMib(2)->address,length-7),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {packetSentToUpperSignal,packetDroppedSignal,Hcf::meshDataAttemptCompletedSignal,
                Hcf::meshPayloadTransmittedSignal,Hcf::meshTerminalRecoverySignal}) network->subscribe(signal,this);
        for (int time : {1,10,50,100,180}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("semantic",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 10) meshManagement(1)->openPeer(meshMib(2)->address);
        if (step == 50) {
            source("semantic-prime",8);
            for (int i=0;i<3;i++) source(("semantic-bad"+std::to_string(i)).c_str(),2304);
        }
        if (step == 100) {
            std::cout << "semantic result scenario=" << par("scenario").intValue() << " delivered=" << delivered << " drops=" << droppedBodies
                << " samples=" << samples << " failures=" << failures << " ppdus=" << sourcePpdus << endl;
            auto agreement = check_and_cast<MeshAmpduHcf *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->agreement(meshMib(1)->address);
            std::cout << "semantic agreement=" << bool(agreement) << " response=" << (agreement && agreement->getIsAddbaResponseReceived()) << endl;
            ASSERT(delivered == 0 && droppedBodies == 3 && samples == 3 && failures == 0 && sourcePpdus == 1);
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address) == 0);
            source("semantic-prime2",8); // Transfer-complete policy retired the previous agreement.
            for (int i=0;i<3;i++) source(("semantic-valid"+std::to_string(i)).c_str(),2304);
        }
        if (step == 180) {
            std::cout << "semantic final delivered=" << delivered << " drops=" << droppedBodies << " samples=" << samples << " failures=" << failures << " ppdus=" << sourcePpdus << endl;
            // Forwarding sends a copy and disposes each received original via
            // its existing drop owner; distinguish those three later disposals
            // from the three invalid bodies checked before any DATA delivery.
            ASSERT(delivered == 3 && samples == 6 && failures == 0 && sourcePpdus == 2 && droppedBodies == 6);
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address) == 0);
            emit(registerSignal("meshAmpduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        if (signal == Hcf::meshTerminalRecoverySignal) ASSERT(false);
        auto packet = dynamic_cast<const Packet *>(object);
        if (!packet || std::string(packet->getName()).find("semantic-") == std::string::npos) return;
        if (std::string(packet->getName()).find("semantic-prime") != std::string::npos) return;
        if (signal == packetSentToUpperSignal && source == meshInterface(2)->getSubmodule("mac") && packet->getDataLength() == B(2304)) delivered++;
        if (signal == packetDroppedSignal && (source == meshInterface(1)->getSubmodule("mac")->getSubmodule("ds")
                || source == meshInterface(1)->getSubmodule("mac"))) droppedBodies++;
        if (source != meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")) return;
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            auto attempt = check_and_cast<MeshDataAttempt *>(details);
            ASSERT(!attempt->terminalTimeout); samples++; failures += !attempt->acknowledged;
        }
        if (signal == Hcf::meshPayloadTransmittedSignal) {
            std::cout << "semantic PPDU at " << simTime() << " count=" << check_and_cast<MeshPayloadTransmission *>(details)->msduCount << endl;
            ASSERT(check_and_cast<MeshPayloadTransmission *>(details)->msduCount == 3); sourcePpdus++;
        }
    }
};
Define_Module(MeshAmpduSemanticDriver);

#endif
