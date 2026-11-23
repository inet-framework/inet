// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERGROUPEXPIRYSUPPORT_H
#define INET_TEST_MESHPOWERGROUPEXPIRYSUPPORT_H

class GroupExpiryDriver : public cSimpleModule, public cListener
{
    int groupsOnAir = 0;
    int groupDrops = 0;
    int individualsOnAir = 0;
    bool dtimEnded = false;
    TimPowerFixture *power(int station) {
        return check_and_cast<TimPowerFixture *>(meshInterface(station)->getSubmodule("powerSavePolicy"));
    }
    TimMacFixture *mac() { return check_and_cast<TimMacFixture *>(meshInterface(0)->getSubmodule("mac")); }
    Hcf *hcf() { return check_and_cast<Hcf *>(mac()->getSubmodule("hcf")); }
    void source(const char *name, MacAddress dest, int tid = 0) {
        sendDirect(fixtureMsdu(name, dest, tid), meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void initialize() override {
        mac()->subscribe(registerSignal("packetSentToLower"), this);
        hcf()->subscribe(registerSignal("packetDropped"), this);
        meshManagement(0)->subscribe(Ieee80211MgmtMesh::meshBeaconTransmissionEndSignal, this);
        for (int ms : {50, 70, 80, 103, 109, 140})
            scheduleAt(SimTime(ms, SIMTIME_MS), new cMessage("group-expiry", ms));
    }
    void receiveSignal(cComponent *, simsignal_t, double, cObject *) override {
        if (simTime() > SimTime(100, SIMTIME_MS)) dtimEnded = true;
    }
    void receiveSignal(cComponent *, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        auto packet = dynamic_cast<Packet *>(object);
        if (!packet) return;
        auto name = std::string(packet->getName());
        if (name == "barrier-expiring-group") {
            if (signal == registerSignal("packetDropped")) {
                ASSERT(simTime() == SimTime(110, SIMTIME_MS));
                ++groupDrops;
            }
            else ++groupsOnAir;
        }
        if (name == "barrier-waiting-vo" && signal == registerSignal("packetSentToLower")) {
            ASSERT(groupDrops == 1 && simTime() >= SimTime(110, SIMTIME_MS));
            ++individualsOnAir;
        }
    }
    void handleMessage(cMessage *event) override {
        int ms = event->getKind(); delete event;
        if (ms == 50) source("barrier-training", meshMib(2)->address);
        else if (ms == 70) power(1)->requestMode(meshMib(0)->address, MeshPowerMode::DEEP_SLEEP);
        else if (ms == 80) {
            mac()->holdGroups = true;
            source("barrier-expiring-group", MacAddress::BROADCAST_ADDRESS);
        }
        else if (ms == 103) {
            ASSERT(dtimEnded && power(0)->groupsCompleted == 0);
            source("barrier-waiting-vo", meshMib(2)->address, 6);
        }
        else if (ms == 109) ASSERT(groupsOnAir == 0 && groupDrops == 0 && individualsOnAir == 0);
        else {
            ASSERT(groupsOnAir == 0 && groupDrops == 1 && individualsOnAir == 1);
            for (const auto& frame : hcf()->getMeshBufferedFrames())
                ASSERT(!frame.receiver.isMulticast() || frame.kind == MeshBufferedFrame::Kind::TIMING);
            emit(registerSignal("timComplete"), 1L);
        }
    }
};
namespace power_group_expiry_registration { Define_Module(::GroupExpiryDriver); }
#endif
