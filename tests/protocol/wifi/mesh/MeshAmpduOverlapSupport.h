// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUOVERLAPSUPPORT_H
#define INET_TEST_MESHAMPDUOVERLAPSUPPORT_H

#include "inet/linklayer/ieee80211/mac/Rx.h"

class MeshAmpduOverlapDriver : public cSimpleModule, public cListener
{
    bool released = false;
    int overlaps = 0, postChecks = 0, navReservations = 0, navExpiries = 0;
    int delivered = 0, incomingDelivered = 0, sourceSamples = 0, sourcePpdus = 0, bars = 0, blockAcks = 0;
    simtime_t overlapEnd = -1, navEnd = -1;
    MeshAmpduHcf *hcf(int station) const { return check_and_cast<MeshAmpduHcf *>(meshInterface(station)->getSubmodule("mac")->getSubmodule("hcf")); }
    void source(int station, const char *name, int destination, int length) {
        sendDirect(phyMsdu(name,meshMib(destination)->address,length-7),meshInterface(station)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
  protected:
    void initialize() override {
        auto network = getSimulation()->getSystemModule();
        for (auto signal : {packetSentToUpperSignal,transmissionEndedSignal,receptionEndedSignal,Hcf::meshPayloadTransmittedSignal,Hcf::meshDataAttemptCompletedSignal})
            network->subscribe(signal,this);
        meshInterface(0)->getSubmodule("mac")->getSubmodule("rx")->subscribe(Rx::navChangedSignal,this);
        for (int time : {1,10,40,50,200}) scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("overlap",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 10) meshManagement(1)->openPeer(meshMib(2)->address);
        if (step == 40) {
            int destination = par("scenario").intValue() == 12 ? 0 : 2;
            source(1,"incoming-prime",destination,8);
            for (int i=0;i<3;i++) source(1,("incoming-overlap"+std::to_string(i)).c_str(),destination,2304);
        }
        if (step == 50) {
            int destination = par("scenario").intValue() == 12 ? 0 : 2;
            ASSERT(hcf(1)->agreement(meshMib(destination)->address));
            ASSERT(hcf(1)->agreement(meshMib(destination)->address)->getIsAddbaResponseReceived());
            source(0,"prime",2,8);
            for (int i=0;i<3;i++) source(0,("aggregate-member"+std::to_string(i)).c_str(),2,7);
        }
        if (step == 1000) {
            ASSERT(!hcf(0)->responseWaitRunning());
            ASSERT(check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric"))->getErrorEstimate(meshMib(1)->address) == 0);
            postChecks++;
        }
        if (step == 200) {
            std::cout << "overlap result released=" << released << " overlaps=" << overlaps << " post=" << postChecks
                << " delivered=" << delivered << " ppdus=" << sourcePpdus << " samples=" << sourceSamples
                << " bars=" << bars << " nav=" << navReservations << "/" << navExpiries << " incoming=" << incomingDelivered << endl;
            ASSERT(released && overlaps == 1 && postChecks == 1 && delivered == 3);
            ASSERT(sourcePpdus == 1 && sourceSamples == 0 && bars == 1);
            ASSERT(blockAcks == (par("scenario").intValue() == 12 ? 1 : 0));
            ASSERT(!hcf(0)->responseWaitRunning() && !hcf(0)->recovering(meshMib(1)->address,0));
            if (par("scenario").intValue() == 10) ASSERT(navReservations >= 1 && navExpiries >= 1 && incomingDelivered == 0);
            if (par("scenario").intValue() == 11) ASSERT(navReservations == 0 && incomingDelivered == 0);
            if (par("scenario").intValue() == 12) ASSERT(navReservations == 0 && incomingDelivered == 3);
            emit(registerSignal("meshAmpduComplete"),1L);
        }
    }
    void receiveSignal(cComponent *, simsignal_t signal, const SimTime& value, cObject *) override {
        Enter_Method_Silent();
        if (signal != Rx::navChangedSignal) return;
        if (simTime() == overlapEnd && value > 0) {
            navReservations++; navEnd = simTime()+value;
        }
        if (value == 0 && simTime() == navEnd) navExpiries++;
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        auto packet = dynamic_cast<const Packet *>(object);
        if (signal == receptionEndedSignal) packet = dynamic_cast<const Packet *>(details);
        if (signal == transmissionEndedSignal) packet = check_and_cast<const Ieee80211Transmission *>(object)->getPacket();
        if (!packet) return;
        std::string name = packet->getName();
        if (signal == Hcf::meshPayloadTransmittedSignal && source == hcf(0) && name.find("aggregate-member") != std::string::npos) {
            ASSERT(!released && check_and_cast<MeshPayloadTransmission *>(details)->msduCount == 3);
            sourcePpdus++;
            released = true;
            par("releaseIncoming").setBoolValue(true);
            hcf(1)->resumeContention();
        }
        if (signal == Hcf::meshDataAttemptCompletedSignal && source == hcf(0) && name.find("aggregate-member") != std::string::npos)
            sourceSamples++;
        if (signal == receptionEndedSignal && source == meshInterface(0)->getSubmodule("radio") && released
                && name.find("incoming-overlap") != std::string::npos && packet->getTag<Ieee80211ModeInd>()->getTxVector().ampdu) {
            ASSERT(overlaps == 0 && hcf(0)->responseWaitRunning() && !hcf(0)->responseTimerScheduled());
            overlapEnd = simTime(); overlaps++;
            scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("wait-released",1000));
        }
        if (signal == packetSentToUpperSignal && packet->getDataLength() == B(7)
                && source == meshInterface(2)->getSubmodule("mac") && name.find("aggregate-member") != std::string::npos)
            delivered++;
        if (signal == packetSentToUpperSignal && packet->getDataLength() == B(2304)
                && source == meshInterface(0)->getSubmodule("mac") && name.find("incoming-overlap") != std::string::npos)
            incomingDelivered++;
        if (signal == transmissionEndedSignal && source == meshInterface(0)->getSubmodule("radio")) {
            auto actual = packet->getTag<Ieee80211ModeReq>();
            if (!actual->getTxVector().ampdu && !actual->getTxVector().singleMpdu) {
                auto phy = Ieee80211Radio::peekIeee80211PhyHeaderAtFront(packet);
                auto type = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength())->getType();
                if (type == ST_BLOCKACK_REQ) bars++;
                if (type == ST_BLOCKACK) blockAcks++;
            }
        }
    }
};
Define_Module(MeshAmpduOverlapDriver);

#endif
