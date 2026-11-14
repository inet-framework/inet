// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAMPDUMETRICSUPPORT_H
#define INET_TEST_MESHAMPDUMETRICSUPPORT_H

class MeshMetricHoldMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t start) const override {
        if (!Ieee80211MeshMac::isFrameTransmissionEligible(packet,start)) return false;
        if (std::string(packet->getName()) == "metric-hold")
            return getSimulation()->getSystemModule()->getSubmodule("driver")->par("releaseHold");
        return true;
    }
};
Define_Module(MeshMetricHoldMac);

class MeshAmpduMetricDriver : public cSimpleModule, public cListener
{
    int delivered = 0, ppdus = 0, releasedPpdus = 0, releasedAttempts = 0;
    std::vector<bool> attempts;
    AirtimeLinkMetric *metric() { return check_and_cast<AirtimeLinkMetric *>(meshInterface(0)->getSubmodule("linkMetric")); }
    void source(const char *name, int length) {
        sendDirect(phyMsdu(name,meshMib(1)->address,length-7),meshInterface(0)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
  protected:
    void initialize() override {
        for (auto signal : {Hcf::meshDataAttemptCompletedSignal,Hcf::meshPayloadTransmittedSignal,packetSentToUpperSignal})
            getParentModule()->subscribe(signal,this);
        for (int step : {1,50,100,150}) scheduleAt(SimTime(step,SIMTIME_MS),new cMessage("aggregate-metric",step));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 50) {
            source("metric-prime",120); // Exceeds A-MSDU policy bound; primes ADDBA alone.
            for (int i=0;i<3;i++) source(("metric-aggregate"+std::to_string(i)).c_str(),7);
            source("metric-aggregate-plain",120);
            source("metric-hold",7); // Existing queue predicate keeps the agreement until the cost checkpoint.
        }
        if (step == 1000) ASSERT(metric()->getPayloadEstimate(meshMib(1)->address) == (ppdus == 1 ? 11.0/8 : 85.0/64));
        if (step == 100) {
            std::cout << "aggregate metric n=" << metric()->getPayloadEstimate(meshMib(1)->address)
                << " ef=" << metric()->getErrorEstimate(meshMib(1)->address) << " ppdus=" << ppdus << endl;
            ASSERT(delivered == 4 && ppdus == 2 && attempts == std::vector<bool>({true,false,true}));
            ASSERT(metric()->getPayloadEstimate(meshMib(1)->address) == 85.0/64);
            ASSERT(metric()->getErrorEstimate(meshMib(1)->address) == 7.0/64);
            auto timing = check_and_cast<IMeshLinkTiming *>(meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf"))->getMeshLinkTiming(meshMib(1)->address);
            int id = meshMib(0)->getMeshConfiguration().pathSelectionMetricId;
            // Independent formula: ID1 disregards n, ID2 amortizes overhead only.
            double n = id == 1 ? 1 : 85.0/64;
            double seconds = (timing.overhead.dbl()/n+8192/timing.rate.get())/(1-7.0/64);
            auto expected = uint32_t(std::ceil(seconds/(id == 1 ? 0.00001024 : 0.00000001)));
            ASSERT(metric()->getMetric(meshMib(1)->address) == expected);
            auto reverse = check_and_cast<AirtimeLinkMetric *>(meshInterface(1)->getSubmodule("linkMetric"));
            ASSERT(reverse->getPayloadEstimate(meshMib(0)->address) == 1 && reverse->getErrorEstimate(meshMib(0)->address) == 0);
            par("releaseHold").setBoolValue(true);
            // A new upper arrival reconsiders existing queued work through the
            // ordinary production path; no test access to contention internals.
            source("metric-after-checkpoint",9);
        }
        if (step == 150) {
            ASSERT(releasedPpdus == 1 && releasedAttempts == 1);
            ASSERT(metric()->getPayloadEstimate(meshMib(1)->address) == 1); // Completed transfer tears down eligibility.
            ASSERT(metric()->getErrorEstimate(meshMib(1)->address) == 49.0/512);
            emit(registerSignal("aggregateMetricComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override {
        Enter_Method_Silent();
        auto packet = dynamic_cast<Packet *>(object);
        if (packet && source == meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")
                && (std::string(packet->getName()).find("metric-hold") != std::string::npos || std::string(packet->getName()).find("metric-after-checkpoint") != std::string::npos)) {
            if (signal == Hcf::meshPayloadTransmittedSignal) {
                ASSERT(check_and_cast<MeshPayloadTransmission *>(details)->msduCount == 2); releasedPpdus++;
            }
            if (signal == Hcf::meshDataAttemptCompletedSignal) {
                auto attempt = check_and_cast<MeshDataAttempt *>(details);
                ASSERT(attempt->acknowledged && !attempt->terminalTimeout); releasedAttempts++;
            }
            return;
        }
        if (!packet || std::string(packet->getName()).find("metric-aggregate") == std::string::npos) return;
        if (signal == packetSentToUpperSignal && source == meshInterface(1)->getSubmodule("mac")) delivered++;
        if (source != meshInterface(0)->getSubmodule("mac")->getSubmodule("hcf")) return;
        if (signal == Hcf::meshDataAttemptCompletedSignal) {
            auto attempt = check_and_cast<MeshDataAttempt *>(details);
            ASSERT(!attempt->terminalTimeout); attempts.push_back(attempt->acknowledged);
        }
        if (signal == Hcf::meshPayloadTransmittedSignal) {
            ASSERT(check_and_cast<MeshPayloadTransmission *>(details)->msduCount == (ppdus == 0 ? 4 : 1)); ppdus++;
            scheduleAt(simTime()+SimTime(1,SIMTIME_NS),new cMessage("aggregate-count",1000));
        }
    }
};
Define_Module(MeshAmpduMetricDriver);

#endif
