// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERRETRYTIMSUPPORT_H
#define INET_TEST_MESHPOWERRETRYTIMSUPPORT_H

class RetryTimDriver : public cSimpleModule, public cListener
{
    int retryPpdus = 0;
    int awakePpdus = 0;
    int sleepingPpdus = 0;
    int retryDeliveries = 0;
    int samples = 0;
    bool retainedTim = false;
    bool clearedTim = false;
    SequenceNumberCyclic firstSequence;
    std::vector<uint8_t> firstBody;
    TimMacFixture *mac() { return check_and_cast<TimMacFixture *>(meshInterface(0)->getSubmodule("mac")); }
    Hcf *hcf() { return check_and_cast<Hcf *>(mac()->getSubmodule("hcf")); }
    TimPowerFixture *power(int station) { return check_and_cast<TimPowerFixture *>(meshInterface(station)->getSubmodule("powerSavePolicy")); }
    Edcaf *owner() { return check_and_cast<Edca *>(hcf()->getSubmodule("edca"))->getEdcaf(AC_BE); }
    void source(const char *name, int station) {
        sendDirect(fixtureMsdu(name, meshMib(station)->address, 0), meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    Packet *retryOriginal() {
        auto frames = owner()->getInProgressFrames();
        for (int i = 0; i < frames->getLength(); ++i)
            if (std::string(frames->getFrames(i)->getName()) == "tim-retained-retry") return frames->getFrames(i);
        return nullptr;
    }
    void initialize() override {
        auto radio = meshInterface(0)->getSubmodule("radio");
        radio->subscribe(registerSignal("transmissionEnded"), this);
        mac()->subscribe(registerSignal("packetSentToLower"), this);
        meshInterface(1)->getSubmodule("mac")->subscribe(registerSignal("packetSentToUpper"), this);
        hcf()->subscribe(Hcf::meshDataAttemptCompletedSignal, this);
        for (int ms : {50, 80, 83, 85, 90, 100, 110, 120, 140, 170, 260})
            scheduleAt(SimTime(ms, SIMTIME_MS), new cMessage("retry-tim", ms));
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            if (simTime() >= SimTime(80, SIMTIME_MS)) ++samples;
            return;
        }
        if (signal == registerSignal("transmissionEnded")) {
            auto transmission = check_and_cast<const Ieee80211Transmission *>(object);
            if (std::string(transmission->getPacket()->getName()) == "tim-retained-retry" && retryPpdus == 1)
                mac()->holdRetry = true;
            return;
        }
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet) return;
        auto name = std::string(packet->getName());
        if (signal == registerSignal("packetSentToUpper")) {
            if (name == "tim-retained-retry") ++retryDeliveries;
            return;
        }
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        if (name == "tim-retained-retry") {
            auto data = packet->peekAtFront<Ieee80211DataHeader>();
            auto bytes = packet->peekDataAt<BytesChunk>(data->getChunkLength(), packet->getDataLength() - data->getChunkLength() - B(4))->getBytes();
            if (retryPpdus == 0) { firstSequence = data->getSequenceNumber(); firstBody = bytes; }
            else {
                ASSERT(simTime() >= SimTime(140, SIMTIME_MS));
                ASSERT(data->getRetry() && data->getSequenceNumber() == firstSequence && bytes == firstBody);
            }
            ++retryPpdus;
        }
        if (name == "same-ac-awake-successor") {
            ASSERT(simTime() < SimTime(140, SIMTIME_MS) && retryOriginal() != nullptr);
            ++awakePpdus;
        }
        if (name == "same-ac-sleeping-head") { ASSERT(simTime() >= SimTime(140, SIMTIME_MS)); ++sleepingPpdus; }
        if (header->getType() == ST_BEACON && simTime() > SimTime(100, SIMTIME_MS)) {
            const auto& tim = packet->peekDataAt<Ieee80211BeaconFrame>(header->getChunkLength())->getTim();
            int index = 1025 / 8 - (tim.bitmapControl & 254);
            bool bit = index >= 0 && index < tim.bitmapLength && (tim.partialVirtualBitmap[index] & 2);
            if (simTime() < SimTime(110, SIMTIME_MS)) {
                auto retry = retryOriginal();
                ASSERT(bit && retry != nullptr && awakePpdus == 0 && sleepingPpdus == 0);
                int buffered = 0;
                for (const auto& frame : hcf()->getMeshBufferedFrames())
                    if (frame.receiver == meshMib(1)->address
                            && (frame.kind == MeshBufferedFrame::Kind::PAYLOAD || frame.kind == MeshBufferedFrame::Kind::MANAGEMENT)) {
                        ASSERT(frame.id == retry->getId());
                        ++buffered;
                    }
                ASSERT(buffered == 1); // The retained retry is B's sole TIM-producing BU.
                ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->remote == MeshPowerMode::DEEP_SLEEP);
                retainedTim = true;
            }
            else if (simTime() > SimTime(200, SIMTIME_MS)) { ASSERT(!bit); clearedTim = true; }
        }
    }
    void handleMessage(cMessage *event) override {
        int ms = event->getKind(); delete event;
        if (ms == 50) { source("retry-training-b", 1); source("retry-training-c", 2); }
        else if (ms == 80) { emit(registerSignal("retryStart"), 1L); source("tim-retained-retry", 1); }
        else if (ms == 83) {
            auto packet = retryOriginal(); ASSERT(packet && retryPpdus == 1);
            ASSERT(owner()->getRecoveryProcedure()->getRetryCount(packet, packet->peekAtFront<Ieee80211DataHeader>()) == 1);
        }
        else if (ms == 85) power(1)->requestMode(meshMib(0)->address, MeshPowerMode::DEEP_SLEEP);
        else if (ms == 90) {
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->remote == MeshPowerMode::DEEP_SLEEP);
            ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->local == MeshPowerMode::DEEP_SLEEP);
        }
        else if (ms == 110) {
            ASSERT(retainedTim);
            mac()->holdData = true;
            source("same-ac-sleeping-head", 1);
            source("same-ac-awake-successor", 2);
            scheduleAt(simTime() + SimTime(1, SIMTIME_NS), new cMessage("same-ac-queued", 91));
        }
        else if (ms == 91) {
            auto queue = owner()->getPendingQueue();
            ASSERT(queue->getNumPackets() == 2);
            ASSERT(std::string(queue->getPacket(0)->getName()) == "same-ac-sleeping-head");
            ASSERT(std::string(queue->getPacket(1)->getName()) == "same-ac-awake-successor");
            mac()->holdData = false; hcf()->meshPowerEligibilityChanged();
        }
        else if (ms == 100) ASSERT(awakePpdus == 0 && retryPpdus == 1 && sleepingPpdus == 0 && retryDeliveries == 1);
        else if (ms == 120) ASSERT(awakePpdus == 1 && retryPpdus == 1 && sleepingPpdus == 0 && retryDeliveries == 1);
        else if (ms == 140) {
            ASSERT(retainedTim && retryOriginal() != nullptr);
            mac()->holdRetry = false;
            power(0)->opportunity(meshMib(1)->address, SimTime(165, SIMTIME_MS));
        }
        else if (ms == 170) ASSERT(retryPpdus == 2 && sleepingPpdus == 1 && retryOriginal() == nullptr && retryDeliveries == 1);
        else {
            ASSERT(retainedTim && clearedTim && retryPpdus == 2 && awakePpdus == 1 && sleepingPpdus == 1 && samples == 4);
            emit(registerSignal("timComplete"), 1L);
        }
    }
};
namespace power_retry_tim_registration { Define_Module(::RetryTimDriver); }
#endif
