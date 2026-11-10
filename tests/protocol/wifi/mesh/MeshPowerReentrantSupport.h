// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERREENTRANTSUPPORT_H
#define INET_TEST_MESHPOWERREENTRANTSUPPORT_H

class PowerReentrantDriver : public cSimpleModule, public cListener
{
    bool armed = false;
    int nested = 0;
    int nullPpdus = 0;
    int64_t oldId = -1;
    int scenario() const { return par("scenario"); }
    AwakePowerFixture *power() {
        return check_and_cast<AwakePowerFixture *>(meshInterface(0)->getSubmodule("powerSavePolicy"));
    }
    PowerHoldMac *mac() { return check_and_cast<PowerHoldMac *>(meshInterface(0)->getSubmodule("mac")); }
    Hcf *hcf() { return check_and_cast<Hcf *>(mac()->getSubmodule("hcf")); }
    MeshPowerMode local() { return meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local; }
    void initialize() override {
        meshMib(0)->subscribe(Ieee80211Mib::meshPowerModeChangedSignal, this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionStarted"), this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionEnded"), this);
        hcf()->subscribe(registerSignal("packetDequeued"), this);
        for (int ms : {5, 10, 70, 80, 90}) scheduleAt(SimTime(ms, SIMTIME_MS), new cMessage("reentrant", ms));
    }
    void receiveSignal(cComponent *, simsignal_t, long, cObject *) override {
        Enter_Method_Silent();
        if (armed && scenario() != 4 && local() == MeshPowerMode::ACTIVE) {
            armed = false;
            ++nested;
            if (scenario() == 1 || scenario() == 3) {
                for (const auto& entry : meshMib(0)->getMeshPeerPowerStates()) ASSERT(entry.second.local == MeshPowerMode::ACTIVE);
                ASSERT(!meshMib(0)->getMeshGroupPowerSaveLevel());
            }
            if (scenario() == 2 || scenario() == 3) power()->requestActiveForAllPeers();
            else power()->requestMode(meshMib(scenario() == 1 ? 2 : 1)->address, MeshPowerMode::LIGHT_SLEEP);
        }
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        if (signal == registerSignal("packetDequeued")) {
            auto packet = check_and_cast<Packet *>(object);
            if (scenario() == 4 && armed && packet->getId() == oldId) {
                armed = false; ++nested;
                power()->requestMode(meshMib(1)->address, MeshPowerMode::LIGHT_SLEEP);
            }
            return;
        }
        auto transmission = check_and_cast<const physicallayer::Ieee80211Transmission *>(object);
        auto packet = transmission->getPacket();
        auto phy = physicallayer::Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
        auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
        if (header->getType() == ST_QOS_NULL) {
            if (signal == registerSignal("transmissionEnded")) {
                if (scenario() == 4 && nullPpdus == 1) mac()->holdNull = true;
                return;
            }
            ++nullPpdus;
            if (simTime() >= SimTime(80, SIMTIME_MS)) {
                auto null = packet->peekDataAt<Ieee80211MeshDataHeader>(phy->getChunkLength());
                ASSERT(null->getPowerMgmt() == (scenario() != 2 && scenario() != 3));
                ASSERT(!null->getMeshPowerSaveLevel());
                ASSERT(null->getReceiverAddress().isMulticast() == (scenario() == 2 || scenario() == 3));
            }
        }
    }
    void handleMessage(cMessage *event) override {
        int ms = event->getKind(); delete event;
        auto peer = meshMib(1)->address;
        if (ms == 5) meshManagement(0)->openPeer(peer);
        else if (ms == 10) { if (scenario() >= 1 && scenario() <= 3) meshManagement(0)->openPeer(meshMib(2)->address); }
        else if (ms == 70) {
            emit(registerSignal("powerStart"), 1L);
            power()->requestMode(peer, MeshPowerMode::DEEP_SLEEP);
            if (scenario() >= 1 && scenario() <= 3) power()->requestMode(meshMib(2)->address, MeshPowerMode::DEEP_SLEEP);
        }
        else if (ms == 80) {
            ASSERT(local() == (scenario() == 4 ? MeshPowerMode::ACTIVE : MeshPowerMode::DEEP_SLEEP));
            ASSERT(nullPpdus == (scenario() >= 1 && scenario() <= 3 ? 2 : 1));
            if (scenario() == 4) {
                for (const auto& frame : hcf()->getMeshBufferedFrames())
                    if (frame.kind == MeshBufferedFrame::Kind::POWER_CONTROL) oldId = frame.id;
                ASSERT(oldId != -1);
            }
            mac()->holdNull = true;
            armed = true;
            if (scenario() == 1 || scenario() == 3) power()->requestActiveForAllPeers();
            else power()->requestMode(peer, MeshPowerMode::ACTIVE);
            ASSERT(nested == 1 && local() == MeshPowerMode::ACTIVE);
            if (scenario() != 2 && scenario() != 3)
                ASSERT(power()->getRequestedMode(meshMib(scenario() == 1 ? 2 : 1)->address) == MeshPowerMode::LIGHT_SLEEP);
            int pending = 0;
            for (const auto& frame : hcf()->getMeshBufferedFrames())
                if (frame.kind == MeshBufferedFrame::Kind::POWER_CONTROL) ++pending;
            ASSERT(pending == 1); // Outer command cannot enqueue an uncorrelated extra Null.
            mac()->holdNull = false;
            hcf()->meshPowerEligibilityChanged();
        }
        else {
            ASSERT(nested == 1 && nullPpdus == (scenario() >= 1 && scenario() <= 3 ? 3 : 2));
            ASSERT(local() == (scenario() == 0 || scenario() == 4 ? MeshPowerMode::LIGHT_SLEEP : MeshPowerMode::ACTIVE));
            if (scenario() == 1) {
                ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(2)->address)->local == MeshPowerMode::LIGHT_SLEEP);
                ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->remote == MeshPowerMode::DEEP_SLEEP);
                ASSERT(meshMib(2)->findMeshPeerPowerState(meshMib(0)->address)->remote == MeshPowerMode::LIGHT_SLEEP);
            }
            else ASSERT(meshMib(1)->findMeshPeerPowerState(meshMib(0)->address)->remote == local());
            ASSERT(check_and_cast<PowerForwardingFixture *>(mac()->getSubmodule("ds"))->nextIdentity() == 0);
            emit(registerSignal("powerComplete"), 1L);
        }
    }
};
namespace power_reentrant_registration { Define_Module(::PowerReentrantDriver); }
#endif
