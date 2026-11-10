// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPHYSUPPORT_H
#define INET_TEST_MESHPHYSUPPORT_H
#include "mesh/MeshForwardingSupport.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mesh/AirtimeLinkMetric.h"
#include "inet/linklayer/ieee80211/mesh/MeshDataAttempt.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmitter.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INarrowbandSignalAnalogModel.h"
#include "inet/common/packet/dissector/PacketDissector.h"

using namespace inet::physicallayer;

inline Packet *phyMsdu(const char *name, MacAddress destination, int extraBytes = 0)
{
    auto packet = fixtureMsdu(name,destination);
    packet->removeAtFront<BytesChunk>(packet->getDataLength());
    // A complete LLC UI header with an unassigned SAP keeps this PHY fixture
    // payload opaque, including during a full production packet dissection.
    std::vector<uint8_t> bytes = {0x7e,0x7e,3,0xde,0xad,0xbe,0xef};
    bytes.insert(bytes.end(),extraBytes,0xab);
    packet->insertAtBack(makeShared<BytesChunk>(bytes));
    return packet;
}

class MeshMemberCaptureAudit : public PacketDissector::ICallback
{
  public:
    std::vector<uint8_t> member;
    int intervals = 0;
    bool incorrect = false;
    bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return protocol != &Protocol::ieee80211Mac; }
    void startProtocolDataUnit(const Protocol *) override {}
    void endProtocolDataUnit(const Protocol *) override {}
    void markIncorrect() override { incorrect = true; }
    void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override {
        if (protocol == &Protocol::ieee80211Mac) {
            intervals++;
            Packet encoded("selected-member",chunk);
            member = encoded.peekAllAsBytes()->getBytes();
        }
    }
};

inline uint32_t independentMeshFcs(const std::vector<uint8_t>& bytes, size_t offset, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = offset; i < offset + length; i++) {
        crc ^= bytes[i];
        for (int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320U : 0);
    }
    return ~crc;
}

class MeshPhyDriver : public cSimpleModule, public cListener
{
    int delivered[2] = {}, transmitted[2] = {}, acknowledged[2] = {};
    int received[2] = {}, duplicateAcks = 0, beacons = 0;
    int bareMacObservations[2] = {};
    std::unique_ptr<Ieee80211Channel> requestedAllocation;
    bool differentAllocationRejected = false;
    bool isData(const Packet *packet) const {
        return std::string(packet->getName()) == "phy-forward" || std::string(packet->getName()) == "phy-reverse";
    }
    Hz expectedCenter() const {
        int width = par("dataWidth"), primary = par("primary");
        return MHz(width == 160 ? 5250 : width == 80 ? (primary == 0 ? 5210 : 5290)
            : width == 40 ? (primary == 0 ? 5190 : 5270) : primary == 0 ? 5180 : 5280);
    }
    int station(cComponent *source, const char *submodule) const {
        for (int i = 0; i < 2; i++)
            if (source == meshInterface(i)->getSubmodule(submodule)) return i;
        return -1;
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {transmissionStartedSignal, receptionEndedSignal, packetSentToUpperSignal, packetSentToLowerSignal, Hcf::meshDataAttemptCompletedSignal})
            network->subscribe(signal, this);
        scheduleAt(SimTime(1,SIMTIME_MS), new cMessage("open",0));
        scheduleAt(SimTime(50,SIMTIME_MS), new cMessage("forward",1));
        scheduleAt(SimTime(80,SIMTIME_MS), new cMessage("reverse",2));
        scheduleAt(SimTime(120,SIMTIME_MS), new cMessage("check",3));
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            auto packet = check_and_cast<Packet *>(object);
            if (!isData(packet)) return;
            auto outcome = check_and_cast<MeshDataAttempt *>(details);
            ASSERT(outcome->acknowledged && !outcome->terminalTimeout);
            ASSERT(packet->getDataLength() == B(49+par("extraBytes").intValue()));
            ASSERT(packet->getTag<PacketProtocolTag>()->getFrontOffset() == b(0) && packet->getTag<PacketProtocolTag>()->getBackOffset() == b(0));
            int from = outcome->peer == meshMib(1)->address ? 0 : 1;
            acknowledged[from]++;
            return;
        }
        if (signal == packetSentToUpperSignal) {
            auto packet = dynamic_cast<Packet *>(object);
            int to = station(source,"mac");
            if (to >= 0 && packet && isData(packet)) {
                std::vector<uint8_t> expected = {0x7e,0x7e,3,0xde,0xad,0xbe,0xef};
                expected.insert(expected.end(),par("extraBytes").intValue(),0xab);
                ASSERT(packet->peekDataAsBytes()->getBytes() == expected);
                delivered[to]++;
            }
            return;
        }
        if (signal == packetSentToLowerSignal) {
            auto packet = dynamic_cast<Packet *>(object);
            int from = station(source,"mac");
            if (from >= 0 && packet && isData(packet)) {
                ASSERT(packet->getDataLength() == B(49+par("extraBytes").intValue()));
                ASSERT(packet->getTag<PacketProtocolTag>()->getFrontOffset() == b(0) && packet->getTag<PacketProtocolTag>()->getBackOffset() == b(0));
                bareMacObservations[from]++;
            }
            return;
        }
        int index = station(source,"radio");
        if (index < 0) return;
        if (signal == receptionEndedSignal) {
            auto packet = dynamic_cast<Packet *>(details);
            if (packet && isData(packet)) {
                ASSERT(!packet->hasBitError());
                auto actual = packet->getTag<Ieee80211ModeInd>()->getTxVector();
                ASSERT(packet->findTag<Ieee80211ModeReq>() == nullptr);
                ASSERT(actual.bandwidth == MHz(par("dataWidth").intValue()));
                ASSERT(packet->getTag<PacketProtocolTag>()->getFrontOffset() == b(0));
                ASSERT(packet->getTag<PacketProtocolTag>()->getBackOffset() == b(0));
                MeshMemberCaptureAudit audit;
                std::unique_ptr<Packet> captured(packet->dup());
                PacketDissector(ProtocolDissectorRegistry::getInstance(),audit).dissectPacket(captured.get());
                ASSERT(!audit.incorrect && audit.intervals == 1 && audit.member.size() == size_t(49+par("extraBytes").intValue()));
                received[index]++;
            }
            return;
        }
        auto transmission = check_and_cast<const Ieee80211Transmission *>(object);
        auto packet = transmission->getPacket();
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto mode = transmission->getMode();
        auto vector = transmission->getTxVector();
        auto analog = check_and_cast<const INarrowbandSignalAnalogModel *>(transmission->getAnalogModel());
        if (!isData(packet)) {
            auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
            if (header->getType() == ST_BEACON) {
                ASSERT(vector.bandwidth == MHz(20) && vector.format == Ieee80211TxVector::Format::NON_HT);
                auto beacon = packet->peekDataAt<Ieee80211BeaconFrame>(phy->getChunkLength()+header->getChunkLength());
                ASSERT(beacon->getHtOperationPresent() && beacon->getHtOperation().primaryChannel == (par("primary").intValue() == 0 ? 36 : 56));
                beacons++;
            }
            if (header->getType() == ST_ACK && vector.bandwidth > MHz(20)) {
                ASSERT(vector.format == Ieee80211TxVector::Format::NON_HT_DUPLICATE && !vector.singleMpdu);
                ASSERT(vector.mode->getDataMode()->getBandwidth() == MHz(20));
                ASSERT(vector.bandwidth == MHz(par("dataWidth").intValue()));
                ASSERT(analog->getCenterFrequency() == expectedCenter());
                ASSERT(transmission->getDuration() == mode->getDuration(LENGTH_ACK));
                duplicateAcks++;
            }
            return;
        }
        auto radio = check_and_cast<Ieee80211Radio *>(meshInterface(index)->getSubmodule("radio"));
        ASSERT(transmission->getChannel() == radio->getOperatingChannel());
        ASSERT(packet->getTag<Ieee80211ChannelReq>()->getChannel() == radio->getOperatingChannel());
        if (index == 0) {
            ASSERT(requestedAllocation && requestedAllocation.get() != transmission->getChannel());
            auto canonical = radio->getOperatingChannel();
            Ieee80211Channel different(canonical->getBand(),canonical->getChannelNumber() == 0 ? 1 : 0,canonical->getAllocationWidth());
            std::unique_ptr<Packet> rejected(packet->dup());
            rejected->getTagForUpdate<Ieee80211ChannelReq>()->setChannel(&different);
            try {
                auto factory = check_and_cast<const Ieee80211Transmitter *>(radio->getTransmitter());
                delete factory->createTransmission(radio,rejected.get(),simTime());
            }
            catch (const cRuntimeError&) { differentAllocationRejected = true; }
            ASSERT(differentAllocationRejected);
        }
        int nss = index == 0 ? par("streams").intValue() : 1;
        ASSERT(mode->getDataMode()->getNumberOfSpatialStreams() == nss);
        ASSERT(analog->getBandwidth() == MHz(par("dataWidth").intValue()) && analog->getCenterFrequency() == expectedCenter());
        ASSERT(vector.bandwidth == analog->getBandwidth());
        ASSERT((mode->isVhtShortGuardInterval() || mode->isHtShortGuardInterval()) == par(index == 0 ? "shortGi" : "reverseShortGi").boolValue());
        ASSERT(par("vht").boolValue() ? mode->getVhtMcsIndex() == 7 : mode->getHtMcsIndex() == 8*nss-1);
        auto bytes = packet->peekDataAt<BytesChunk>(phy->getChunkLength(), phy->getLengthField())->getBytes();
        size_t offset = 0, memberLength = bytes.size();
        if (par("vht")) {
            ASSERT(dynamicPtrCast<const Ieee80211VhtPhyHeader>(phy));
            ASSERT(vector.singleMpdu && vector.format == Ieee80211TxVector::Format::VHT);
            int word = bytes[0] | bytes[1]<<8;
            ASSERT((word & 3) == 1 && bytes[3] == 0x4E);
            memberLength = (word >> 4) | ((word & 12) << 10);
            offset = 4;
            ASSERT(bytes.size() == 4 + memberLength + (4-memberLength%4)%4);
        }
        else ASSERT(!vector.singleMpdu && vector.format == Ieee80211TxVector::Format::HT_MIXED);
        ASSERT(memberLength >= 4);
        auto fcs = independentMeshFcs(bytes,offset,memberLength-4);
        for (int byte = 0; byte < 4; byte++) ASSERT(bytes[offset+memberLength-4+byte] == ((fcs>>(8*byte))&255));
        ASSERT(transmission->getDuration() == mode->getDuration(B(bytes.size())));
        MeshMemberCaptureAudit audit;
        std::unique_ptr<Packet> captured(packet->dup());
        PacketDissector(ProtocolDissectorRegistry::getInstance(),audit).dissectPacket(captured.get());
        ASSERT(!audit.incorrect && audit.intervals == 1);
        ASSERT(audit.member == std::vector<uint8_t>(bytes.begin()+offset,bytes.begin()+offset+memberLength));
        transmitted[index]++;
    }
    void handleMessage(cMessage *message) override {
        int kind = message->getKind(); delete message;
        if (kind == 0) {
            ASSERT(!meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
            meshManagement(0)->openPeer(meshMib(1)->address);
        }
        if (kind == 1 || kind == 2) {
            int from = kind-1, to = 1-from;
            ASSERT(meshMib(from)->isMeshPeerEstablished(meshMib(to)->address));
            auto committed = meshMib(from)->findMeshPeerPhyCapabilities(meshMib(to)->address);
            ASSERT(committed && committed->htPresent && committed->vhtPresent == par("vht").boolValue());
            ASSERT(committed->htOperation.primaryChannel == par("primary").intValue());
            ASSERT(committed->ht.rxMcsSupported[8] == (from == 0 && par("streams").intValue() == 2));
            ASSERT(committed->ht.txRxMcsSetNotEqual == (par("streams").intValue() == 2));
            if (par("vht")) {
                ASSERT(committed->ht.supportedChannelWidths.count(MHz(40)));
                ASSERT(committed->vht.rxMcs[0] >= 7 && committed->vht.txMcs[0] >= 7);
                ASSERT(committed->vht.width160 == (par("dataWidth").intValue() == 160));
                ASSERT((committed->vht.rxMcs[1] >= 7) == (from == 0 && par("streams").intValue() == 2));
                ASSERT((committed->vht.txMcs[1] >= 7) == (from == 1 && par("streams").intValue() == 2));
            }
            auto timing = check_and_cast<IMeshLinkTiming *>(meshInterface(from)->getSubmodule("mac")->getSubmodule("hcf"));
            auto metric = check_and_cast<AirtimeLinkMetric *>(meshInterface(from)->getSubmodule("linkMetric"));
            auto nominal = timing->getMeshLinkTiming(meshMib(to)->address);
            int id = meshMib(from)->getMeshConfiguration().pathSelectionMetricId;
            ASSERT(metric->getMetric(meshMib(to)->address) == AirtimeLinkMetric::computeMetric(id,nominal.overhead,nominal.rate,0));
            auto packet = phyMsdu(kind == 1 ? "phy-forward" : "phy-reverse",meshMib(to)->address,par("extraBytes").intValue());
            if (kind == 1) {
                auto radio = check_and_cast<Ieee80211Radio *>(meshInterface(from)->getSubmodule("radio"));
                requestedAllocation = std::make_unique<Ieee80211Channel>(*radio->getOperatingChannel());
                packet->addTag<Ieee80211ChannelReq>()->setChannel(requestedAllocation.get());
            }
            sendDirect(packet,meshInterface(from)->getSubmodule("llc")->gate("lowerLayerOut"));
        }
        if (kind == 3) {
            for (int i = 0; i < 2; i++) ASSERT(delivered[i] == 1 && received[i] == 1 && transmitted[i] == 1 && acknowledged[i] == 1 && bareMacObservations[i] == 1);
            ASSERT(beacons > 0);
            ASSERT(differentAllocationRejected);
            ASSERT(par("dataWidth").intValue() == 20 || duplicateAcks == 2);
            emit(registerSignal("meshPhyComplete"),1L);
        }
    }
  public:
    using cListener::finish;
};
Define_Module(MeshPhyDriver);
#endif
