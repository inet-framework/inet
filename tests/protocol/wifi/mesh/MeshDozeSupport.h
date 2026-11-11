// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHDOZESUPPORT_H
#define INET_TEST_MESHDOZESUPPORT_H

#include "mesh/MeshPhySupport.h"
#include "inet/linklayer/ieee80211/mesh/MeshPowerSave.h"

class DozePowerFixture : public MeshPowerSave
{
  protected:
    bool isActivationSupported() const override { return true; } // Explicit pre-P3 test profile, no MPSP engine.
  public:
    simtime_t ownWindowEnd() const { return ownAwEnd; }
    simtime_t postWindowEnd() const { return postAwEnd; }
    bool awaitingOwnBeacon() const { return ownRendezvous; }
    void opportunity(MacAddress peer,simtime_t end) { Enter_Method("opportunity"); openAwakeOpportunity(peer,end); }
    simtime_t opportunityEnd(MacAddress peer) const {
        auto found=awakeUntil.find(peer);
        return found==awakeUntil.end() ? SIMTIME_ZERO : found->second;
    }
    std::string describeObligations() const {
        std::ostringstream out;
        out << "awake=" << awakeRequired << " own=" << ownAwEnd << " post=" << postAwEnd
            << " ownWait=" << ownRendezvous << " maintenance=" << maintenanceOrdinal
            << " maintenanceAt=" << management->getOwnTbtt(maintenanceOrdinal)
            << " transitions=" << transitions.size();
        for (const auto& entry : awakeUntil) out << " opportunity=" << entry.first << ":" << entry.second;
        for (const auto& entry : peerWake) out << " peer=" << entry.first << ":" << entry.second;
        out << " inventory=" << buffers->getMeshBufferedFrames().size()
            << " exchange=" << radioControl->hasMeshRadioActivity()
            << " peering=" << management->getMeshWakeSchedule().peeringPending;
        return out.str();
    }
};
Define_Module(DozePowerFixture);

class DozeDriver : public cSimpleModule, public cListener
{
    int sleeps[2] = {}, wakes[2] = {};
    IRadio::RadioMode lastStable[2] = {IRadio::RADIO_MODE_OFF, IRadio::RADIO_MODE_OFF};
    simtime_t wakeStarted[2], lastWake[2];
    int beaconReady = 0, tbttReady = 0, delivered = 0, samples = 0, errors = 0, awakeChanges = 0;
    DozePowerFixture *power(int i) const {
        return check_and_cast<DozePowerFixture *>(meshInterface(i)->getSubmodule("powerSavePolicy"));
    }
    IRadio *radio(int i) const { return check_and_cast<IRadio *>(meshInterface(i)->getSubmodule("radio")); }
    void at(simtime_t time, int kind, short priority = 0) {
        auto event = new cMessage("doze-check", kind);
        event->setSchedulingPriority(priority);
        scheduleAt(time, event);
    }
    int station(cComponent *source, const char *part) const {
        for (int i = 0; i < 2; i++) if (source == meshInterface(i)->getSubmodule(part)) return i;
        return -1;
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {IRadio::radioModeChangedSignal, transmissionStartedSignal,
                packetSentToUpperSignal, Hcf::meshDataAttemptCompletedSignal,
                MeshPowerSave::meshAwakeRequirementChangedSignal}) network->subscribe(signal, this);
        at(SimTime(1, SIMTIME_MS), 0);
        at(SimTime(100, SIMTIME_MS), 1);
        // These observers predate the later wake timers and run before the
        // higher-priority same-time beacon/radio work can supply readiness.
        for (auto time : {SimTime(102400, SIMTIME_US), SimTime(179200, SIMTIME_US), SimTime(204800, SIMTIME_US)})
            at(time, 2, -100);
        at(SimTime(130, SIMTIME_MS), 3);
        at(SimTime(250, SIMTIME_MS), 4);
        at(SimTime(270, SIMTIME_MS), 5);
        at(SimTime(350, SIMTIME_MS), 6);
    }
    void receiveSignal(cComponent *source,simsignal_t signal,bool value,cObject *) override {
        if (signal!=MeshPowerSave::meshAwakeRequirementChangedSignal) return;
        int from=station(source,"powerSavePolicy");
        ASSERT(from>=0 && power(from)->requiresAwake()==value);
        awakeChanges++;
    }
    void receiveSignal(cComponent *source, simsignal_t, long value, cObject *) override {
        int from = station(source, "radio");
        if (from < 0) return;
        auto mode = static_cast<IRadio::RadioMode>(value);
        if (mode == IRadio::RADIO_MODE_SWITCHING) {
            if (lastStable[from] == IRadio::RADIO_MODE_SLEEP) wakeStarted[from] = simTime();
            return;
        }
        if (mode == IRadio::RADIO_MODE_SLEEP) {
            for (const auto& entry : meshMib(from)->getMeshPeerPowerStates()) ASSERT(entry.second.local!=MeshPowerMode::ACTIVE);
            ASSERT(simTime() >= power(from)->ownWindowEnd() && simTime() >= power(from)->postWindowEnd());
            ASSERT(!power(from)->awaitingOwnBeacon());
            sleeps[from]++;
        }
        if (mode == IRadio::RADIO_MODE_TRANSCEIVER && lastStable[from] == IRadio::RADIO_MODE_SLEEP) {
            if (!par("zeroDelay").boolValue()) ASSERT(simTime() - wakeStarted[from] == SimTime(200, SIMTIME_US));
            lastWake[from] = simTime();
            wakes[from]++;
        }
        lastStable[from] = mode;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            auto result = check_and_cast<MeshDataAttempt *>(details);
            samples++;
            errors += !result->acknowledged;
            return;
        }
        if (signal == packetSentToUpperSignal) {
            auto packet = dynamic_cast<Packet *>(object);
            if (packet && station(source, "mac") == 1 && std::string(packet->getName()) == "after-doze") delivered++;
            return;
        }
        int from = station(source, "radio");
        if (from < 0 || signal != transmissionStartedSignal) return;
        auto packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
        if (header->getType() == ST_QOS_NULL)
            std::cout << "doze Null TX station=" << from << " t=" << simTime() << std::endl;
        if (header->getType() == ST_BEACON && simTime() > SimTime(100, SIMTIME_MS) && simTime() < SimTime(250, SIMTIME_MS)) {
            auto other = radio(1 - from)->getRadioModeState();
            ASSERT(other && !other->transition && other->mode == IRadio::RADIO_MODE_TRANSCEIVER);
            auto body = packet->peekDataAt<Ieee80211BeaconFrame>(phy->getChunkLength() + header->getChunkLength());
            ASSERT(body->getTim().bitmapLength == 1 && body->getTim().partialVirtualBitmap[0] == 0);
            beaconReady++;
        }
    }
    void handleMessage(cMessage *event) override {
        int kind = event->getKind(); delete event;
        if (kind == 0) {
            meshManagement(0)->openPeer(meshMib(1)->address);
            meshManagement(1)->openPeer(meshMib(0)->address);
        }
        else if (kind == 1) {
            ASSERT(meshMib(0)->isMeshPeerEstablished(meshMib(1)->address));
            ASSERT(meshMib(1)->isMeshPeerEstablished(meshMib(0)->address));
            power(0)->requestMode(meshMib(1)->address, MeshPowerMode::LIGHT_SLEEP);
            power(1)->requestMode(meshMib(0)->address, MeshPowerMode::LIGHT_SLEEP);
        }
        else if (kind == 2) {
            for (int i = 0; i < 2; i++) {
                auto state = radio(i)->getRadioModeState();
                ASSERT(state && !state->transition && state->mode == IRadio::RADIO_MODE_TRANSCEIVER);
                ASSERT(lastWake[i] < simTime());
                tbttReady++;
            }
        }
        else if (kind == 3) {
            for (int i = 0; i < 2; i++) {
                std::cout << "doze state station=" << i << " local=" << int(meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address)->local)
                    << " remote=" << int(meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address)->remote)
                    << " sleeps=" << sleeps[i] << " wakes=" << wakes[i] << " " << power(i)->describeObligations() << std::endl;
                ASSERT(meshMib(i)->findMeshPeerPowerState(meshMib(1-i)->address)->local == MeshPowerMode::LIGHT_SLEEP);
                // A contended indication can commit after the 102.4ms beacon.
                // Its first sleep is then real, but its first wake is at the
                // next required TBTT. The final check still requires two cycles.
                ASSERT(sleeps[i] > 0);
                int before=awakeChanges;
                power(i)->reviewAwakeState();
                ASSERT(awakeChanges==before); // A completed no-op decision publishes no change.
            }
            ASSERT(samples == 0 && errors == 0);
        }
        else if (kind == 4) {
            power(0)->requestMode(meshMib(1)->address, MeshPowerMode::ACTIVE);
            power(1)->requestMode(meshMib(0)->address, MeshPowerMode::ACTIVE);
        }
        else if (kind == 5) {
            check_and_cast<IMeshForwarding *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("ds"))
                ->processSourceMsdu(phyMsdu("after-doze", meshMib(1)->address));
        }
        else {
            ASSERT(delivered == 1 && samples == 1 && errors == 0 && tbttReady == 6 && beaconReady >= 3);
            ASSERT(awakeChanges>=4);
            for (int i = 0; i < 2; i++) ASSERT(sleeps[i] >= 2 && wakes[i] >= 2);
            emit(registerSignal("dozeComplete"), 1L);
        }
    }
};
Define_Module(DozeDriver);

#endif
