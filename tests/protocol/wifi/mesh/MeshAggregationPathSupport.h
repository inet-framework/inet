// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHAGGREGATIONPATHSUPPORT_H
#define INET_TEST_MESHAGGREGATIONPATHSUPPORT_H

class MeshRankingMac : public Ieee80211MeshMac
{
  public:
    bool isFrameTransmissionEligible(const Packet *packet, simtime_t start) const override {
        return std::string(packet->getName()).find("rank-hold") == std::string::npos
            && Ieee80211MeshMac::isFrameTransmissionEligible(packet,start);
    }
};
Define_Module(MeshRankingMac);

class MeshAggregationPathDriver : public cSimpleModule, public cListener
{
    int delivered = 0;
    uint32_t viaPlain = 0, viaAggregate = 0;
    uint32_t discoverySequence = 0;
    std::vector<MacAddress> rootChoices;
    uint32_t selectedCostAtProcessing = 0;
    void source(int from, int to, const char *name, int size) {
        sendDirect(phyMsdu(name,meshMib(to)->address,size-7),meshInterface(from)->getSubmodule("llc")->gate("lowerLayerOut"));
    }
    void burst(int from, int to) {
        source(from,to,"rank-prime",120);
        for (int i=0;i<4;i++) source(from,to,("rank-warm"+std::to_string(i)).c_str(),7);
        source(from,to,"rank-hold",7);
    }
  protected:
    void initialize() override {
        getParentModule()->subscribe(packetSentToUpperSignal,this);
        getParentModule()->subscribe(registerSignal("hwmpTestHandoff"),this);
        getParentModule()->subscribe(Hwmp::pathUpdatedSignal,this);
        for (int time : {1,5,9,13,40,50,60,70,80,90,100,110,145,150,165})
            scheduleAt(SimTime(time,SIMTIME_MS),new cMessage("ranking",time));
    }
    void handleMessage(cMessage *message) override {
        int step = message->getKind(); delete message;
        if (step == 1) meshManagement(0)->openPeer(meshMib(1)->address);
        if (step == 5) meshManagement(0)->openPeer(meshMib(2)->address);
        if (step == 9) meshManagement(1)->openPeer(meshMib(3)->address);
        if (step == 13) meshManagement(2)->openPeer(meshMib(3)->address);
        if (step == 40) source(0,1,"plain-warm",7);
        if (step == 50) source(1,0,"plain-warm",7);
        if (step == 60) source(1,3,"plain-warm",7);
        if (step == 70) source(3,1,"plain-warm",7);
        if (step == 80) burst(0,2);
        if (step == 90) burst(2,0);
        if (step == 100) burst(2,3);
        if (step == 110) burst(3,2);
        if (step == 145) {
            for (auto link : {std::make_pair(0,2),std::make_pair(2,0),std::make_pair(2,3),std::make_pair(3,2)}) {
                auto metric = check_and_cast<AirtimeLinkMetric *>(meshInterface(link.first)->getSubmodule("linkMetric"));
                ASSERT(metric->getPayloadEstimate(meshMib(link.second)->address) == 11.0/8);
                ASSERT(metric->getErrorEstimate(meshMib(link.second)->address) == 0);
            }
            viaPlain = outgoingMetric(0,1)+outgoingMetric(1,3);
            viaAggregate = outgoingMetric(0,2)+outgoingMetric(2,3);
            bool id2 = meshMib(0)->getMeshConfiguration().pathSelectionMetricId == 2;
            ASSERT(id2 ? viaAggregate < viaPlain : viaPlain <= viaAggregate); // ID1 can quantize both alternatives to a tie.
            ASSERT(!hwmp(0)->findPath(meshMib(3)->address)); // No DATA refresh of the expired old control information.
        }
        if (step == 150) source(0,3,"rank-selected",11);
        if (step == 165) {
            auto path = hwmp(0)->getPathInfo(meshMib(3)->address);
            bool id2 = meshMib(0)->getMeshConfiguration().pathSelectionMetricId == 2;
            std::cout << "rank result id=" << int(meshMib(0)->getMeshConfiguration().pathSelectionMetricId) << " plain=" << viaPlain
                << " aggregate=" << viaAggregate << " delivered=" << delivered << endl;
            if (path) std::cout << "rank path next=" << path->nextHop << " valid=" << path->validated << " hops=" << int(path->hops)
                << " metric=" << path->metric << " expiry=" << path->expiry << endl;
            else std::cout << "rank path absent attempts=" << hwmp(0)->getDiscoveryAttempts(meshMib(3)->address) << endl;
            ASSERT(delivered == 1 && path && path->validated && path->hops == 2);
            ASSERT(path->nextHop == meshMib(id2 ? 2 : 1)->address && path->metric == (id2 ? viaAggregate : viaPlain));
            ASSERT(path->metric == selectedCostAtProcessing);
            ASSERT(rootChoices == (id2 ? std::vector<MacAddress>{meshMib(1)->address,meshMib(2)->address} : std::vector<MacAddress>{meshMib(1)->address}));
            emit(registerSignal("hwmpAggregationPathComplete"),1L);
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, bool, cObject *details) override {
        Enter_Method_Silent();
        if (signal == Hwmp::pathUpdatedSignal && source == hwmp(0) && simTime() >= SimTime(150,SIMTIME_MS)) {
            auto change = check_and_cast<Hwmp::PathChange *>(details);
            if (change->destination == meshMib(3)->address && change->path && change->path->validated) {
                int relay = change->path->nextHop == meshMib(1)->address ? 1 : 2;
                selectedCostAtProcessing = outgoingMetric(0,relay)+outgoingMetric(relay,3);
                ASSERT(change->path->metric == selectedCostAtProcessing);
            }
        }
    }
    void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *) override {
        Enter_Method_Silent();
        auto packet = dynamic_cast<const Packet *>(object);
        if (packet && signal == registerSignal("hwmpTestHandoff") && simTime() >= SimTime(145,SIMTIME_MS)) {
            auto body = packet->peekAtFront<Ieee80211HwmpFrame>();
            int station = -1;
            for (int i=0;i<4;i++) if (source == meshManagement(i)) station = i;
            for (size_t i=0;i<body->getPreqsArraySize();i++) {
                auto preq = body->getPreqs(i);
                std::cout << "rank PREQ " << simTime() << " " << source->getFullPath() << " origin=" << preq.getOriginator()
                    << " sn=" << preq.getOriginatorSequenceNumber() << " metric=" << preq.getMetric() << endl;
                if (preq.getOriginator() == meshMib(0)->address) {
                    if (station == 0) { ASSERT(discoverySequence == 0); discoverySequence = preq.getOriginatorSequenceNumber(); }
                    if (station == 1 || station == 2) {
                        auto metric = check_and_cast<AirtimeLinkMetric *>(meshInterface(station)->getSubmodule("linkMetric"));
                        ASSERT(preq.getOriginatorSequenceNumber() == discoverySequence && preq.getMetric() == outgoingMetric(station,0));
                        ASSERT(metric->getErrorEstimate(meshMib(0)->address) == 0 && metric->getPayloadEstimate(meshMib(0)->address) == (station == 2 ? 11.0/8 : 1));
                    }
                }
            }
            for (size_t i=0;i<body->getPrepsArraySize();i++) {
                auto prep = body->getPreps(i);
                std::cout << "rank PREP " << simTime() << " " << source->getFullPath() << " target=" << prep.target
                    << " sn=" << prep.targetSequenceNumber << " metric=" << prep.metric << endl;
                if (prep.target == meshMib(3)->address && prep.originator == meshMib(0)->address) {
                    if (station == 3) {
                        auto reverse = hwmp(3)->getPathInfo(meshMib(0)->address);
                        ASSERT(reverse && reverse->sequence && *reverse->sequence == discoverySequence);
                        int relay = reverse->nextHop == meshMib(1)->address ? 1 : 2;
                        ASSERT(reverse->metric == outgoingMetric(3,relay)+outgoingMetric(relay,0));
                        rootChoices.push_back(reverse->nextHop);
                    }
                    if (station == 1 || station == 2) ASSERT(prep.metric == outgoingMetric(station,3));
                }
            }
        }
        if (signal == packetSentToUpperSignal && packet && source == meshInterface(3)->getSubmodule("mac") && packet->getDataLength() == B(11)) delivered++;
    }
};
Define_Module(MeshAggregationPathDriver);

#endif
