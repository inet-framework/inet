//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessClassifier.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessScheduler.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessClassifier);

void AirtimeFairnessClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        subqueueModuleType = par("subqueueModuleType").stdstringValue();
        WATCH_EXPR("numStations", (int)addressToIndex.size());
    }
}

MacAddress AirtimeFairnessClassifier::getReceiverAddress(Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    return header->getReceiverAddress();
}

int AirtimeFairnessClassifier::classifyPacket(Packet *packet)
{
    MacAddress address = getReceiverAddress(packet);
    auto it = addressToIndex.find(address);
    if (it != addressToIndex.end())
        return it->second;
    // first frame for this receiver -> next free branch; create it on demand if the enclosing
    // compound has no pre-wired branch for this index (dynamic case), else reuse the pre-wired
    // one (static case, where the vectors were sized in NED).
    int index = (int)addressToIndex.size();
    if (index >= gateSize("out"))
        createBranch(index);
    addressToIndex[address] = index;
    return index;
}

int AirtimeFairnessClassifier::createBranch(int index)
{
    cModule *parent = getParentModule();
    // grow the per-station sub-queue and gate submodule vectors
    parent->setSubmoduleVectorSize("queue", index + 1);
    parent->setSubmoduleVectorSize("gate", index + 1);
    // create the sub-queue (parametrized type) and the airtime-fairness gate
    cModule *subqueue = cModuleType::get(subqueueModuleType.c_str())->create("queue", parent, index);
    cModule *gateModule = cModuleType::get("inet.linklayer.ieee80211.mac.queue.AirtimeFairnessGate")->create("gate", parent, index);
    // grow my own output gate vector and wire classifier.out[index] -> queue.in
    setGateSize("out", index + 1);
    cGate *classifierOut = gate("out", index);
    classifierOut->connectTo(subqueue->gate("in"));
    outputGates.push_back(classifierOut);
    queueing::PassivePacketSinkRef consumer;
    consumer.reference(classifierOut, false);
    consumers.push_back(consumer);
    queueing::ActivePacketSinkRef collector;
    collector.reference(classifierOut, false);
    collectors.push_back(collector);
    // wire queue.out -> gate.in
    subqueue->gate("out")->connectTo(gateModule->gate("in"));
    // grow scheduler.in and wire gate.out -> scheduler.in[index]
    auto scheduler = check_and_cast<AirtimeFairnessScheduler *>(parent->getSubmodule("scheduler"));
    scheduler->setGateSize("in", scheduler->gateSize("in") + 1);
    cGate *schedulerIn = scheduler->gate("in", scheduler->gateSize("in") - 1);
    gateModule->gate("out")->connectTo(schedulerIn);
    // finalize + initialize the new modules now that the whole chain is connected
    subqueue->finalizeParameters();
    subqueue->buildInside();
    gateModule->finalizeParameters();
    gateModule->buildInside();
    subqueue->callInitialize();
    gateModule->callInitialize();
    // register the new input with the scheduler (matched-pair refs + downstream notify)
    scheduler->addInput(schedulerIn);
    EV_INFO << "Created airtime-fairness branch " << index << EV_ENDL;
    return index;
}

} // namespace ieee80211
} // namespace inet
