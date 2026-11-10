// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERCONDITIONALTIMSUPPORT_H
#define INET_TEST_MESHPOWERCONDITIONALTIMSUPPORT_H

class ConditionalTimDriver : public cSimpleModule, public cListener
{
    bool nonDtim = false;
    bool emptyDtim = false;
    bool actualDuration = false;
    int delivered = 0;
    TimPowerFixture *power(int station) {
        return check_and_cast<TimPowerFixture *>(meshInterface(station)->getSubmodule("powerSavePolicy"));
    }
    void initialize() override {
        meshInterface(0)->getSubmodule("mac")->subscribe(registerSignal("packetSentToLower"), this);
        meshInterface(0)->getSubmodule("radio")->subscribe(registerSignal("transmissionStarted"), this);
        for (int ms : {50, 70, 72, 80, 120, 260})
            scheduleAt(SimTime(ms, SIMTIME_MS), new cMessage("conditional-tim", ms));
    }
    void source(const char *name) {
        sendDirect(fixtureMsdu(name, meshMib(1)->address, 0),
            meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        if (signal == registerSignal("transmissionStarted")) {
            auto transmission = check_and_cast<const Ieee80211Transmission *>(object);
            auto packet = transmission->getPacket();
            auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
            auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
            if (header->getType() == ST_BEACON && simTime() > SimTime(100, SIMTIME_MS)
                    && simTime() < SimTime(110, SIMTIME_MS)) {
                ASSERT(check_and_cast<TimHcfFixture *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->sawConditionalWindowBeforeRate());
                ASSERT(transmission->getEndTime() - transmission->getStartTime() == SimTime(136, SIMTIME_US));
                actualDuration = true;
            }
            return;
        }
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet) return;
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        if (std::string(packet->getName()) == "conditional-buffered") {
            ASSERT(simTime() >= SimTime(120, SIMTIME_MS));
            ++delivered;
        }
        if (header->getType() != ST_BEACON || simTime() < SimTime(100, SIMTIME_MS)) return;
        auto beacon = packet->peekDataAt<Ieee80211BeaconFrame>(header->getChunkLength());
        const auto& tim = beacon->getTim();
        ASSERT(tim.dtimPeriod == 2);
        ASSERT(beacon->getMeshAwakeWindowPresent());
        ASSERT(header->getPowerMgmt()); // All-LIGHT local state: PM1 but PSL0.
        ASSERT(!(beacon->getMeshConfiguration().capability & 64));
        ASSERT(!(tim.bitmapControl & 1)); // No buffered group unit.
        if (simTime() < SimTime(110, SIMTIME_MS)) {
            ASSERT(tim.dtimCount == 1 && tim.bitmapLength == 1 && (tim.bitmapControl & 254) == 128);
            ASSERT(tim.partialVirtualBitmap[0] & 2); // Reporting AID1025, compressed offset128.
            nonDtim = true;
        }
        else if (simTime() > SimTime(200, SIMTIME_MS)) {
            ASSERT(tim.dtimCount == 0 && tim.bitmapLength == 1 && tim.partialVirtualBitmap[0] == 0);
            emptyDtim = true; // DTIM retains its Awake Window with an empty TIM.
        }
    }
    void handleMessage(cMessage *event) override {
        int ms = event->getKind(); delete event;
        if (ms == 50) {
            ASSERT(meshManagement(0)->getLocalAid(meshMib(1)->address) == 1025);
            source("conditional-training");
        }
        else if (ms == 70) power(1)->requestMode(meshMib(0)->address, MeshPowerMode::DEEP_SLEEP);
        else if (ms == 72) power(0)->requestMode(meshMib(1)->address, MeshPowerMode::LIGHT_SLEEP);
        else if (ms == 80) {
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->local == MeshPowerMode::LIGHT_SLEEP);
            ASSERT(meshMib(0)->findMeshPeerPowerState(meshMib(1)->address)->remote == MeshPowerMode::DEEP_SLEEP);
            source("conditional-buffered");
            check_and_cast<TimManagementFixture *>(meshManagement(0))->queueReport(meshMib(1)->address);
        }
        else if (ms == 120) {
            ASSERT(nonDtim && actualDuration && delivered == 0);
            power(0)->opportunity(meshMib(1)->address, SimTime(145, SIMTIME_MS));
        }
        else {
            ASSERT(nonDtim && emptyDtim && actualDuration && delivered == 1);
            emit(registerSignal("timComplete"), 1L);
        }
    }
};
namespace power_conditional_tim_registration { Define_Module(::ConditionalTimDriver); }
#endif
