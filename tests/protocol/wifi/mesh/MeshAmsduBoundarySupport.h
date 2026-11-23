// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMSDUBOUNDARYSUPPORT_H
#define INET_TEST_MESHAMSDUBOUNDARYSUPPORT_H
#include "inet/common/lifecycle/LifecycleController.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.h"
#include "inet/physicallayer/wireless/noise/NoiseSource.h"

class MeshAmsduNoise : public NoiseSource
{
  public:
    void pulse() { Enter_Method("pulse"); cancelEvent(sleepTimer); startTransmission(); }
};
Define_Module(MeshAmsduNoise);

class MeshAmsduHcfProbe : public Hcf
{
  public:
    bool permitsUnsentRetry(Packet *packet) {
        Enter_Method("permitsUnsentRetry");
        PreparedDataTransmission prepared; prepared.frame = packet;
        std::unique_ptr<FrameSequenceContext> context(buildContext(AC_BE));
        return isInitialDataException(context.get(),prepared);
    }
    void reconsider() { Enter_Method("reconsider"); updateContention(); }
};
Define_Module(MeshAmsduHcfProbe);

class MeshAmsduManagement : public Ieee80211MgmtMesh
{
    bool relaxLimits = false;
    bool removeSgi = false;
  protected:
    void sendDown(Packet *packet) override {
        int scenario = getSimulation()->getSystemModule()->getSubmodule("driver")->par("scenario");
        if (getParentModule() == meshInterface(1) && (scenario == 20 || removeSgi)) {
            auto body = dynamicPtrCast<const Ieee80211MeshFrame>(packet->peekAtFront<Chunk>());
            if (body && (body->getHtCapabilitiesPresent() || body->getVhtCapabilitiesPresent())) {
                auto edited = dynamicPtrCast<Ieee80211MeshFrame>(packet->removeAtFront<Chunk>());
                if (edited->getHtCapabilitiesPresent()) {
                    auto ht = edited->getHtCapabilities();
                    if (scenario == 20 && !relaxLimits) ht.maxAmsduLength7935 = false;
                    if (removeSgi) {
                        ht.shortGi20 = ht.shortGi40 = false;
                        for (int mcs=8;mcs<77;mcs++) ht.rxMcsSupported[mcs] = false;
                        ht.rxHighestSupportedDataRate = 0;
                        ht.txRxMcsSetNotEqual = true; ht.txMaxNss = 2;
                    }
                    edited->setHtCapabilities(ht);
                }
                if (edited->getVhtCapabilitiesPresent()) {
                    auto vht = edited->getVhtCapabilities();
                    if (scenario == 20 && !relaxLimits) vht.capabilitiesInfo &= ~3u;
                    if (removeSgi) {
                        vht.capabilitiesInfo &= ~((1u<<5)|(1u<<6));
                        vht.rxMcsMap |= 0xfffc; // Remove optional NSS2+, retaining mandatory NSS1 MCS0..7.
                        vht.rxHighestSupportedDataRate = 0;
                    }
                    edited->setVhtCapabilities(vht);
                }
                packet->insertAtFront(edited);
            }
        }
        Ieee80211MgmtMesh::sendDown(packet);
    }
  public:
    void updateCapability(MacAddress peer, bool sgi) {
        Enter_Method("updateCapability"); relaxLimits = true; removeSgi = sgi;
        sendPeering(peer,MESH_PEERING_OPEN);
    }
};
Define_Module(MeshAmsduManagement);

class MeshAmsduBoundaryDriver;
class MeshAmsduMacProbe : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const override;
};

class MeshAmsduBoundaryDriver : public cSimpleModule, public cListener, public ITx::ICallback, public LifecycleController
{
    int delivered = 0, ordinary = 0, rawDelivered = 0, rawCompletions = 0;
    int localSiblings = 0;
    int removedSequence = -1;
    int secondDelivered = 0, capabilityPublications = 0;
    bool holdCandidate = false, pendingCapability = false, onAirStopScheduled = false;
    const IIeee80211Mode *beforeCapability = nullptr;
    uint32_t beforeCapabilityCost = 0;
    bool observedRetryOverrun = false;
    int aggregateTx[3] = {}, plainTx[3] = {}, payloadEvents[3] = {}, dropSources = 0, rejectedTransit = 0;
    int failed = 0, acked = 0, rawAckCount = 0;
    int perrs = 0;
    simtime_t rawReceivedEnd = -1;
    std::unique_ptr<Packet> templateFrame;
    std::vector<uint8_t> retryBody;
    const Packet *retainedRetry = nullptr;
    bool retryNoiseStarted = false, stopped = false;
    int rawSequence = 1000;
    int scenario() const { return par("scenario"); }
    bool deadlineCase() const { return scenario() == 12 || scenario() == 13; }
    MeshAmsduForwardingProbe *forwarder(int i) { return check_and_cast<MeshAmsduForwardingProbe *>(meshInterface(i)->getSubmodule("mac")->getSubmodule("ds")); }
    Hcf *hcf(int i) { return check_and_cast<Hcf *>(meshInterface(i)->getSubmodule("mac")->getSubmodule("hcf")); }
    AirtimeLinkMetric *metric(int i) { return check_and_cast<AirtimeLinkMetric *>(meshInterface(i)->getSubmodule("linkMetric")); }
    Edcaf *be(int i) { return check_and_cast<Edcaf *>(hcf(i)->getSubmodule("edca")->getSubmodule("edcaf",1)); }
    int batchSize() const { return scenario() == 1 ? 4 : 3; }
    int payloadLength(int i) const {
        if (scenario() == 1) return i == 3 ? 943 : 2304;
        if (scenario() == 9 || scenario() == 17 || scenario() == 18) return 1500;
        if (scenario() == 22) return 500+i;
        return 7+i;
    }
    bool batch(const Packet *packet) const { return std::string(packet->getName()).rfind("sibling",0) == 0; }
    int pendingSources() {
        int count = 0;
        for (int ac=0;ac<4;ac++) {
            auto queue = check_and_cast<Edca *>(hcf(0)->getSubmodule("edca"))->getEdcaf(static_cast<AccessCategory>(ac))->getPendingQueue();
            for (int i=0;i<queue->getNumPackets();i++) {
                auto tag = queue->getPacket(i)->findTag<Ieee80211MeshAggregationTag>();
                count += tag && tag->getPendingSourceIdentity();
            }
        }
        return count;
    }
    void source(const char *name, int length=7, int destination=2, int tid=0) {
        auto packet = phyMsdu(name,meshMib(destination)->address,length-7);
        packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(tid);
        sendDirect(packet,meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void operation(bool start, bool crash=false) {
        LifecycleOperation *operation = start ? static_cast<LifecycleOperation *>(new ModuleStartOperation()) :
            crash ? static_cast<LifecycleOperation *>(new ModuleCrashOperation()) : static_cast<LifecycleOperation *>(new ModuleStopOperation());
        LifecycleOperation::StringMap parameters;
        operation->initialize(forwarder(0),parameters);
        ASSERT(initiateOperation(operation));
    }
    void sendRaw(bool repaired) {
        ASSERT(templateFrame);
        auto original = templateFrame->peekAtFront<Ieee80211MeshDataHeader>();
        auto outer = staticPtrCast<Ieee80211MeshDataHeader>(original->dupShared());
        bool finalHop = scenario() == 7 || scenario() == 8;
        int transmitter = finalHop ? 1 : 0;
        outer->setTransmitterAddress(meshMib(transmitter)->address);
        outer->setReceiverAddress(meshMib(transmitter+1)->address);
        outer->setSequenceNumber(SequenceNumberCyclic(rawSequence++));
        outer->setRetry(false);
        outer->setDurationField(SIMTIME_ZERO);
        std::vector<int> lengths;
        if (scenario() == 7) lengths = {2040,1979}; // Body4059 + native overhead36 = non-HT PSDU4095.
        else if (scenario() == 8) lengths = {1900,repaired ? 1899 : 1900}; // Exact HT lower-tier3839/3840.
        else lengths = {7,8,9};
        bool duplicate = scenario() == 5;
        uint32_t sequence = duplicate ? 1 : 0xabcdef00 + scenario();
        if (scenario() == 4 && !repaired) outer->setAddress3(meshMib(1)->address); // No derived self precursor at B.
        auto frame = std::make_unique<Packet>("raw-siblings",outer);
        for (size_t i=0;i<lengths.size();i++) {
            auto header = makeShared<Ieee80211MeshMsduSubframeHeader>();
            header->setDa(outer->getAddress3()); header->setSa(outer->getAddress4()); header->setLength(lengths[i]);
            if (scenario() == 3 && !repaired && i == 1) header->setLength(2304); // Valid FCS, bounded-invalid complete body.
            auto control = makeShared<Ieee80211MeshControl>();
            control->setSequenceNumber(sequence);
            control->setTtl(scenario() == 6 ? (!repaired && i == 0 ? 1 : 2) : finalHop ? 1 : 3);
            frame->insertAtBack(header); frame->insertAtBack(control);
            std::vector<uint8_t> payload(lengths[i],0xab); payload[0]=payload[1]=0x7e; payload[2]=3;
            frame->insertAtBack(makeShared<BytesChunk>(payload));
            int padding = (4-(20+lengths[i])%4)%4;
            if (i+1<lengths.size() && padding) frame->insertAtBack(makeShared<ByteCountChunk>(B(padding)));
        }
        auto trailer = makeShared<Ieee80211MacTrailer>();
        trailer->setFcsMode(FCS_COMPUTED); trailer->setFcs(0);
        frame->insertAtBack(trailer);
        auto bytes = frame->peekDataAsBytes()->getBytes();
        auto fcs = independentMeshFcs(bytes,0,bytes.size()-4);
        for (int i=0;i<4;i++) bytes[bytes.size()-4+i]=(fcs>>(8*i))&255;
        auto raw = new Packet("raw-siblings",makeShared<BytesChunk>(bytes));
        auto mode = check_and_cast<IMeshLinkTiming *>(hcf(transmitter))->getMeshLinkTiming(meshMib(transmitter+1)->address).dataMode;
        if (finalHop) mode = Ieee80211ModeSet::getModeSet("ac-complete")->getMode(Mbps(24));
        raw->addTag<Ieee80211ModeReq>()->setTxVector(getMpduTxVector(mode,outer));
        raw->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211Mac);
        auto tx = check_and_cast<ITx *>(meshInterface(transmitter)->getSubmodule("mac")->getSubmodule("tx"));
        ASSERT(!tx->hasTransmission());
        tx->transmitFrame(raw,outer,false,this); delete raw;
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {packetSentToUpperSignal,packetSentToLowerSignal,packetDroppedSignal,receptionEndedSignal,transmissionStartedSignal,Hcf::meshPayloadTransmittedSignal,Hcf::meshDataAttemptCompletedSignal})
            network->subscribe(signal,this);
        network->subscribe(registerSignal("rateStateChanged"),this);
        network->subscribe(registerSignal("meshAmsduAllocatorRemoved"),this);
        for (int time : {1,10,50,100,140,145,150,190}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("boundary",time));
        if (scenario() == 19) scheduleAt(SimTime(60,SIMTIME_MS),new cMessage("prime-local-endpoint",60));
        if (scenario() == 20) for (int time : {160,170,175,180}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("aggregation-reset",time));
        if ((scenario() >= 11 && scenario() <= 15) || scenario() == 25) {
            scheduleAt(SimTime(99990,SIMTIME_US),new cMessage("formation-hold",99));
            scheduleAt(SimTime(104,SIMTIME_MS),new cMessage("pending-check",104));
        }
        if (deadlineCase()) {
            scheduleAt(SimTime(101,SIMTIME_MS),new cMessage("second",101));
            scheduleAt(SimTime(102,SIMTIME_MS),new cMessage("third",102));
        }
        if (scenario() == 12) {
            scheduleAt(SimTime(120500,SIMTIME_US),new cMessage("oldest-expired",120));
            scheduleAt(SimTime(122,SIMTIME_MS)+SimTime(1,SIMTIME_NS),new cMessage("all-expired",122));
        }
        if (scenario() == 14 || scenario() == 15) scheduleAt(SimTime(113,SIMTIME_MS),new cMessage("restart",113));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 10) meshManagement(1)->openPeer(meshMib(2)->address);
        if (step == 50) source("prime",7,2,scenario() == 21 ? 4 : 0);
        if (step == 60) source("prime-local",7,1);
        if (step == 99) check_and_cast<MeshAmsduNoise *>(getParentModule()->getSubmodule("formationNoise"))->pulse();
        if (step == 100) {
            ASSERT(forwarder(0)->nextSourceSequence() == (scenario() == 19 ? 2 : 1));
            emit(registerSignal("meshAmsduSeedStart"),1L);
            if (deadlineCase()) source("sibling0",7);
            else for (int i=0;i<batchSize();i++) source(("sibling"+std::to_string(i)).c_str(),payloadLength(i),scenario() == 19 && i == 1 ? 1 : 2,scenario() == 2 && i == 1 ? 3 : 0);
        }
        if (step == 101) source("sibling1",8);
        if (step == 102) source("sibling2",9);
        if (step == 104) {
            if (scenario() == 11) {
                ASSERT(be(1)->getPendingQueue()->getNumPackets() >= 2); // Committed transit units really overlap in the relay queue.
            }
            else {
                ASSERT(pendingSources() == 3 && forwarder(0)->nextSourceSequence() == 1);
                ASSERT(metric(0)->getPayloadEstimate(meshMib(1)->address) == 1);
                if (scenario() == 13) {
                    auto service = check_and_cast<OriginatorQosMacDataService *>(hcf(0)->getSubmodule("originatorMacDataService"));
                    auto first = service->prepareFrame(be(0)->getPendingQueue(),true);
                    auto second = service->prepareFrame(be(0)->getPendingQueue(),true);
                    ASSERT(first && second && first->aggregate && second->aggregate && first->sourceIds.size() == 3);
                    ASSERT(forwarder(0)->nextSourceSequence() == 1 && pendingSources() == 3);
                    ASSERT(service->isPreparationValid(*first,SimTime(110,SIMTIME_MS)));
                    ASSERT(!service->isPreparationValid(*first,SimTime(120,SIMTIME_MS)));
                    auto committed = service->commitPreparedFrame(*first); ASSERT(committed); take(committed);
                    auto lifetime = check_and_cast<Edca *>(hcf(0)->getSubmodule("edca"))->getLifetimeHandler();
                    ASSERT(lifetime->getDeadline(committed) == SimTime(122,SIMTIME_MS));
                    ASSERT(forwarder(0)->nextSourceSequence() == 2 && pendingSources() == 0);
                    auto header = committed->removeAtFront<Ieee80211MeshDataHeader>(); header->setRetry(true); committed->insertAtFront(header);
                    ASSERT(committed->getTag<Ieee80211MeshAggregationTag>()->getInitialMpduLength() == -1);
                    ASSERT(!check_and_cast<MeshAmsduHcfProbe *>(hcf(0))->permitsUnsentRetry(committed));
                    lifetime->forgetFrame(committed->getId()); delete committed;
                    ASSERT(metric(0)->getPayloadEstimate(meshMib(1)->address) == 1 && metric(0)->getErrorEstimate(meshMib(1)->address) == 0);
                }
                if (scenario() == 14 || scenario() == 15) {
                    ASSERT(!check_and_cast<ITx *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("tx"))->hasTransmission());
                    operation(false,scenario() == 15); stopped = true;
                    ASSERT(!forwarder(0)->isSourceActive() && pendingSources() == 0 && dropSources == 3 && forwarder(0)->nextSourceSequence() == 1);
                    ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
                }
                if (scenario() == 25) {
                    ASSERT(!check_and_cast<ITx *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("tx"))->hasTransmission());
                    std::unique_ptr<Packet> pending(be(0)->getPendingQueue()->getPacket(0)->dup());
                    forwarder(0)->deleteModule();
                    ASSERT(removedSequence == 1 && meshInterface(0)->getSubmodule("mac")->getSubmodule("ds") == nullptr);
                    ASSERT(pendingSources() == 0 && dropSources == 3);
                    auto mac = check_and_cast<Ieee80211MeshMac *>(meshInterface(0)->getSubmodule("mac"));
                    ASSERT(!mac->isFrameTransmissionEligible(pending.get(),simTime()));
                    pending->getTagForUpdate<Ieee80211MeshAggregationTag>()->setPendingSourceIdentity(false);
                    mac->finalizeDataFrame(pending.get()); // Committed work never dereferences a missing allocator.
                    ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
                }
            }
        }
        if (step == 113) {
            ASSERT(stopped && payloadEvents[0] == 0 && aggregateTx[0] == 0 && forwarder(0)->nextSourceSequence() == 1);
            operation(true); stopped = false;
        }
        if (step == 120) {
            ASSERT(retainedRetry && be(0)->getInProgressFrames()->containsFrame(const_cast<Packet *>(retainedRetry)));
            auto lifetime = check_and_cast<Edca *>(hcf(0)->getSubmodule("edca"))->getLifetimeHandler();
            ASSERT(lifetime->getDeadline(retainedRetry) == SimTime(122,SIMTIME_MS));
            std::vector<Ieee80211MeshAmsduPart> parts;
            ASSERT(parseMeshAmsdu(retainedRetry,retainedRetry->peekAtFront<Ieee80211MeshDataHeader>(),parts) && parts.size() == 3);
        }
        if (step == 122) ASSERT(be(0)->getInProgressFrames()->getLength() == 0 && payloadEvents[0] == 1 && failed == 1);
        if (step == 225) check_and_cast<MeshAmsduManagement *>(meshManagement(1))->updateCapability(meshMib(0)->address,true);
        if (step == 226) {
            ASSERT(check_and_cast<ITx *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("tx"))->hasTransmission() == (scenario() == 23));
            ASSERT(forwarder(0)->nextSourceSequence() == 2);
            operation(false); stopped = true;
            ASSERT(dropSources == 0 && meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
        }
        if (step == 140 && (scenario() == 23 || scenario() == 24)) {
            ASSERT(stopped && delivered == 3 && aggregateTx[0] == (scenario() == 24 ? 2 : 1));
            ASSERT(metric(0)->getPayloadEstimate(meshMib(1)->address) == (scenario() == 24 ? 47.0/32 : 5.0/4));
            operation(true); stopped = false;
        }
        if (step == 140 && scenario() == 22) {
            ASSERT(observedRetryOverrun && metric(0)->getPayloadEstimate(meshMib(1)->address) == 5.0/4);
            ASSERT(metric(0)->getErrorEstimate(meshMib(1)->address) == 0 && forwarder(0)->nextSourceSequence() == 2);
        }
        if (step == 140 && scenario() >= 3 && scenario() <= 8) sendRaw(false);
        if (step == 145 && (scenario() == 3 || scenario() == 6 || scenario() == 8)) sendRaw(true);
        if (step == 150 && scenario() != 20) {
            if (scenario() == 25) {
                source("deleted-source"); // Explicit unavailable-owner drop, without an exception.
                sendDirect(phyMsdu("ordinary-after-boundary",meshMib(2)->address),meshInterface(1)->getSubmodule("llc")->gate("lowerLayerOut"));
            }
            else source("ordinary-after-boundary",7,2,scenario() == 21 ? 4 : 0);
        }
        if (step == 160) {
            ASSERT(metric(0)->getErrorEstimate(meshMib(1)->address) == 7.0/64);
            ASSERT(metric(0)->getPayloadEstimate(meshMib(1)->address) == 47.0/32);
            beforeCapability = check_and_cast<IMeshLinkTiming *>(hcf(0))->getMeshLinkTiming(meshMib(1)->address).dataMode;
            beforeCapabilityCost = metric(0)->getMetric(meshMib(1)->address); pendingCapability = true;
            check_and_cast<MeshAmsduManagement *>(meshManagement(1))->updateCapability(meshMib(0)->address,false);
        }
        if (step == 170) for (int i=0;i<3;i++) source(("second-sibling"+std::to_string(i)).c_str(),7+i);
        if (step == 175) {
            ASSERT(capabilityPublications == 1 && secondDelivered == 3 && metric(0)->getPayloadEstimate(meshMib(1)->address) == 5.0/4);
            ASSERT(metric(0)->getErrorEstimate(meshMib(1)->address) == 49.0/512 && metric(1)->getPayloadEstimate(meshMib(2)->address) == 47.0/32);
        }
        if (step == 180) source("ordinary-after-boundary");
        if (step == 190) {
            ASSERT(ordinary == 1 && perrs == 0);
            if (scenario() == 1) ASSERT(delivered == 4 && aggregateTx[0] == 1 && aggregateTx[1] == 1);
            if (scenario() == 2 || scenario() == 11) ASSERT(delivered == 3 && aggregateTx[0] == 0 && aggregateTx[1] == 0 && plainTx[0] == 3 && plainTx[1] == 3);
            if (scenario() == 16 || scenario() == 17 || scenario() == 18) {
                ASSERT(delivered == 3 && aggregateTx[0] == 0 && aggregateTx[1] == 0 && plainTx[0] == 3 && plainTx[1] == 3);
                ASSERT(forwarder(0)->nextSourceSequence() == 5 && metric(0)->getPayloadEstimate(meshMib(1)->address) == 1);
            }
            if (scenario() == 19) ASSERT(delivered == 2 && localSiblings == 1 && aggregateTx[0] == 0 && plainTx[0] == 3 && plainTx[1] == 2);
            if (scenario() == 21) ASSERT(delivered == 0 && aggregateTx[0] == 0 && plainTx[0] == 0 && payloadEvents[0] == 0 && forwarder(0)->nextSourceSequence() == 2);
            if (scenario() >= 3 && scenario() <= 8) {
                ASSERT(delivered == 3 && aggregateTx[0] == 1 && aggregateTx[1] >= 1);
                int expected = scenario() == 3 || scenario() == 6 ? 3 : scenario() == 7 || scenario() == 8 ? 2 : 0;
                ASSERT(rawDelivered == expected && rawCompletions == (scenario() == 3 || scenario() == 6 || scenario() == 8 ? 2 : 1));
                if (scenario() == 7 || scenario() == 8) ASSERT(rawAckCount == rawCompletions);
            }
            if (scenario() == 9 || scenario() == 10) ASSERT(delivered == 0 && aggregateTx[0] == 1 && aggregateTx[1] == 0 && rejectedTransit == 1 && payloadEvents[1] == 0);
            if (scenario() == 12) ASSERT(delivered == 3 && aggregateTx[0] == 1 && failed == 1 && acked == 0);
            if (scenario() == 13 || scenario() == 14 || scenario() == 15) ASSERT(delivered == 0 && aggregateTx[0] == 0 && payloadEvents[0] == 0);
            if (scenario() != 12 && scenario() != 20 && scenario() != 22 && scenario() != 24) ASSERT(failed == 0);
            if (scenario() == 20) ASSERT(capabilityPublications == 1 && secondDelivered == 3 && failed == 1);
            if (scenario() == 22) ASSERT(capabilityPublications == 1 && observedRetryOverrun && delivered == 3 && aggregateTx[0] == 2 && failed == 1 && !holdCandidate);
            if (scenario() == 23 || scenario() == 24) ASSERT(delivered == 3 && !stopped && dropSources == 0);
            if (scenario() == 14 || scenario() == 15) ASSERT(dropSources == 3 && forwarder(0)->nextSourceSequence() == 2);
            if (scenario() == 25) ASSERT(removedSequence == 1 && dropSources == 3 && pendingSources() == 0 && aggregateTx[0] == 0 && plainTx[0] == 0 && payloadEvents[0] == 0);
            emit(registerSignal("meshAmsduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *, simsignal_t signal, intval_t value, cObject *) override {
        if (signal == registerSignal("meshAmsduAllocatorRemoved")) {
            removedSequence = value;
            ASSERT(scenario() == 25 && value == 1 && pendingSources() == 0 && dropSources == 3 && !forwarder(0)->isSourceActive());
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, bool value, cObject *) override {
        if (source != meshMib(0) || signal != registerSignal("rateStateChanged") || !value || !pendingCapability) return;
        pendingCapability = false; capabilityPublications++;
        auto mode = check_and_cast<IMeshLinkTiming *>(hcf(0))->getMeshLinkTiming(meshMib(1)->address).dataMode;
        ASSERT(metric(0)->getPayloadEstimate(meshMib(1)->address) == 1);
        if (scenario() == 20) {
            ASSERT(mode == beforeCapability && metric(0)->getErrorEstimate(meshMib(1)->address) == 7.0/64);
            ASSERT(meshMib(0)->getMeshConfiguration().pathSelectionMetricId == 1
                ? metric(0)->getMetric(meshMib(1)->address) == beforeCapabilityCost
                : metric(0)->getMetric(meshMib(1)->address) > beforeCapabilityCost);
        }
        else {
            ASSERT(scenario() == 22 && mode != beforeCapability && !mode->isHtShortGuardInterval() && !mode->isVhtShortGuardInterval());
            ASSERT(metric(0)->getErrorEstimate(meshMib(1)->address) == 0);
            holdCandidate = false;
            check_and_cast<MeshAmsduHcfProbe *>(hcf(0))->reconsider();
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        auto packet = dynamic_cast<Packet *>(object);
        if (signal == receptionEndedSignal) {
            packet = dynamic_cast<Packet *>(details);
            if ((scenario() == 7 || scenario() == 8) && packet && std::string(packet->getName()) == "raw-siblings" && source == meshInterface(2)->getSubmodule("radio") && !packet->hasBitError()) {
                auto mode = packet->getTag<Ieee80211ModeInd>()->getTxVector();
                ASSERT(mode.format == Ieee80211TxVector::NON_HT && !mode.singleMpdu);
                rawReceivedEnd = simTime();
            }
            return;
        }
        if (signal == transmissionStartedSignal) {
            auto transmission = dynamic_cast<const Ieee80211Transmission *>(object);
            if (transmission && source == meshInterface(0)->getSubmodule("radio") && batch(transmission->getPacket()) && scenario() == 23 && !onAirStopScheduled) {
                Enter_Method_Silent(); onAirStopScheduled = true;
                scheduleAt(simTime()+SimTime(5,SIMTIME_US),new cMessage("on-air-owner-stop",226));
            }
            if (transmission && rawReceivedEnd >= SIMTIME_ZERO && source == meshInterface(2)->getSubmodule("radio")) {
                auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(transmission->getPacket());
                if (transmission->getPacket()->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())->getType() == ST_ACK && simTime() == rawReceivedEnd+SimTime(16,SIMTIME_US)) rawAckCount++;
            }
            return;
        }
        if (!packet) return;
        if (signal == packetSentToLowerSignal && simTime() >= SimTime(100,SIMTIME_MS)
                && (source == meshInterface(0)->getSubmodule("mac") || source == meshInterface(1)->getSubmodule("mac") || source == meshInterface(2)->getSubmodule("mac"))) {
            auto header = packet->peekAtFront<Ieee80211MacHeader>();
            auto action = dynamicPtrCast<const Ieee80211MeshActionFrame>(header);
            if (action && action->getCategory() == MESH_ACTION && action->getMeshAction() == MESH_HWMP_PATH_SELECTION)
                perrs += packet->peekDataAt<Ieee80211HwmpFrame>(header->getChunkLength(),packet->getDataLength()-header->getChunkLength()-B(4))->getPerrsArraySize();
        }
        if (signal == packetSentToUpperSignal && source == meshInterface(2)->getSubmodule("mac")) {
            if (batch(packet)) delivered++;
            else if (std::string(packet->getName()) == "ordinary-after-boundary") ordinary++;
            else if (std::string(packet->getName()).rfind("second-sibling",0) == 0) secondDelivered++;
            else if (std::string(packet->getName()) == "raw-siblings" || std::string(packet->getName()) == "mesh-msdu") rawDelivered++;
        }
        if (signal == packetSentToUpperSignal && source == meshInterface(1)->getSubmodule("mac") && batch(packet)) localSiblings++;
        if (signal == packetDroppedSignal) {
            auto pending = packet->findTag<Ieee80211MeshAggregationTag>();
            if (source == hcf(0) && pending && pending->getPendingSourceIdentity()) dropSources++;
            if (source == hcf(1) && batch(packet)) rejectedTransit++;
        }
        if (scenario() == 6 && signal == packetSentToLowerSignal && source == meshInterface(1)->getSubmodule("mac") && std::string(packet->getName()) == "raw-siblings") {
            std::vector<Ieee80211MeshAmsduPart> parts;
            ASSERT(parseMeshAmsdu(packet,packet->peekAtFront<Ieee80211MeshDataHeader>(),parts) && parts.size() == 3);
            for (const auto& part : parts) ASSERT(part.control->getTtl() == 1);
            ASSERT(packet->getTag<Ieee80211MeshAggregationTag>()->getMsduCount() == 3);
        }
        if (!batch(packet)) return;
        if (signal == packetSentToLowerSignal && (source == meshInterface(0)->getSubmodule("mac") || source == meshInterface(1)->getSubmodule("mac"))) {
            int from = source == meshInterface(0)->getSubmodule("mac") ? 0 : 1;
            auto header = packet->peekAtFront<Ieee80211MeshDataHeader>();
            if (header->getAMsduPresent()) {
                std::vector<Ieee80211MeshAmsduPart> parts; ASSERT(parseMeshAmsdu(packet,header,parts));
                ASSERT(parts.size() == size_t(batchSize()));
                if (scenario() == 1) ASSERT(packet->getDataLength() == B(7971)); // Standalone HT upper tier exceeds nested HT member cap.
                if (from == 0 && !templateFrame) {
                    Enter_Method_Silent();
                    templateFrame.reset(packet->dup());
                }
                if (from == 0 && scenario() == 22) {
                    auto bytes = packet->peekDataAsBytes()->getBytes();
                    std::vector<uint8_t> body(bytes.begin()+32,bytes.end()-4);
                    if (retryBody.empty()) {
                        retryBody = body; beforeCapability = packet->getTag<Ieee80211ModeReq>()->getMode();
                        auto timing = check_and_cast<IMeshLinkTiming *>(hcf(0))->getMeshLinkTiming(meshMib(1)->address);
                        ASSERT(beforeCapability->getDataMode()->getNumberOfSpatialStreams() == 2);
                        ASSERT(computeMpduPpduDuration(beforeCapability,packet) + beforeCapability->getSifsTime()
                            + timing.ackMode->getDuration(LENGTH_ACK) <= be(0)->getTxopProcedure()->getLimit());
                    }
                    else {
                        ASSERT(retryBody == body && header->getRetry() && packet->getTag<Ieee80211MeshAggregationTag>()->getInitialMpduLength() == packet->getDataLength().get<B>());
                        auto mode = packet->getTag<Ieee80211ModeReq>()->getMode();
                        auto timing = check_and_cast<IMeshLinkTiming *>(hcf(0))->getMeshLinkTiming(meshMib(1)->address);
                        ASSERT(beforeCapability->getDataMode()->getNumberOfSpatialStreams() == 2 && mode->getDataMode()->getNumberOfSpatialStreams() == 1);
                        ASSERT(mode != beforeCapability && computeMpduPpduDuration(mode,packet) + mode->getSifsTime()
                            + timing.ackMode->getDuration(LENGTH_ACK) > be(0)->getTxopProcedure()->getLimit());
                        observedRetryOverrun = true;
                    }
                }
                aggregateTx[from]++;
            }
            else {
                ASSERT(scenario() == 2 || scenario() == 11 || scenario() == 16 || scenario() == 17 || scenario() == 18 || scenario() == 19);
                auto control = packet->peekDataAt<Ieee80211MeshControl>(header->getChunkLength());
                ASSERT(control->getSequenceNumber() >= 1 && control->getSequenceNumber() <= (scenario() == 19 ? 4 : 3));
                plainTx[from]++;
            }
        }
        if (signal == Hcf::meshPayloadTransmittedSignal) {
            int from = source == hcf(0) ? 0 : 1;
            auto payload = check_and_cast<MeshPayloadTransmission *>(details);
            ASSERT(payload->msduCount == (scenario() == 2 || scenario() == 11 || scenario() == 16 || scenario() == 17 || scenario() == 18 || scenario() == 19 ? 1 : batchSize()));
            payloadEvents[from]++;
        }
        if (signal == Hcf::meshDataAttemptCompletedSignal && source == hcf(0)) {
            auto attempt = check_and_cast<MeshDataAttempt *>(details);
            if (attempt->acknowledged) acked++; else {
                failed++;
                if ((scenario() == 12 || scenario() == 24) && !retryNoiseStarted) {
                    Enter_Method_Silent(); retryNoiseStarted = true;
                    retainedRetry = packet;
                    check_and_cast<MeshAmsduNoise *>(getParentModule()->getSubmodule("retryNoise"))->pulse();
                    if (scenario() == 24) scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("retained-retry-owner-stop",226));
                }
                if (scenario() == 22 && failed == 1) {
                    Enter_Method_Silent(); holdCandidate = true; pendingCapability = true;
                    scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("accepted-sgi-removal",225));
                }
            }
        }
    }
  public:
    bool holdsRetry() const { return holdCandidate; }
    bool prepareTransmissionStart(const Packet *, simtime_t) override { return true; }
    void transmissionCancelled(const Packet *) override { ASSERT(false); }
    void transmissionComplete(Packet *, const Ptr<const Ieee80211MacHeader>&) override { rawCompletions++; }
    using cListener::finish;
};
Define_Module(MeshAmsduBoundaryDriver);

bool MeshAmsduMacProbe::isFrameTransmissionEligible(const Packet *packet, simtime_t plannedStart) const
{
    auto driver = dynamic_cast<MeshAmsduBoundaryDriver *>(getSimulation()->getSystemModule()->getSubmodule("driver"));
    if (getParentModule() == meshInterface(0) && driver && driver->holdsRetry() && std::string(packet->getName()).rfind("sibling",0) == 0)
        return false; // Fixture barrier until the real capability transaction completes.
    return Ieee80211MeshMac::isFrameTransmissionEligible(packet,plannedStart);
}
Define_Module(MeshAmsduMacProbe);
#endif
