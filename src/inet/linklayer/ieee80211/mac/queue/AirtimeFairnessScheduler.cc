//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessScheduler.h"

#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessGate.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessScheduler);

void AirtimeFairnessScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        fairnessEnabled = par("fairnessEnabled");
        resolveGates();
        // The per-station branches are wired up as the stations appear (see
        // ~DynamicClassifier), so watch for an input gate of ours being connected. Only
        // connections made after this point are reported; the ones coming from the NED
        // topology are already in inputGates.
        subscribe(POST_MODEL_CHANGE, this);
        WATCH(cursor);
    }
}

void AirtimeFairnessScheduler::receiveSignal(cComponent *source, simsignal_t signalID, cObject *object, cObject *details)
{
    if (signalID == POST_MODEL_CHANGE) {
        // A connection ending at one of our input gates was just created; the notification
        // is fired on the module owning the path end gate, that is, on us.
        if (auto notification = dynamic_cast<cPostPathCreateNotification *>(object)) {
            cGate *inputGate = notification->pathEndGate;
            if (inputGate->getOwnerModule() == this && !strcmp(inputGate->getBaseName(), "in"))
                addInput(inputGate);
        }
    }
}

void AirtimeFairnessScheduler::resolveGates()
{
    // Resolve the AirtimeFairnessGate feeding each input gate (matched pair). The gate is at
    // the far end of the connection path (branch[i].gate.out --> ... --> in[i]), which is not
    // the same as the previous gate: the branch is a compound module.
    gates.resize(inputGates.size());
    for (size_t i = 0; i < inputGates.size(); i++)
        gates[i] = check_and_cast<AirtimeFairnessGate *>(inputGates[i]->getPathStartGate()->getOwnerModule());
}

void AirtimeFairnessScheduler::addInput(cGate *inputGate)
{
    Enter_Method("addInput");
    inputGates.push_back(inputGate);
    queueing::PassivePacketSourceRef provider;
    provider.reference(inputGate, false);
    providers.push_back(provider);
    queueing::ActivePacketSourceRef producer;
    producer.reference(inputGate, false);
    producers.push_back(producer);
    gates.push_back(check_and_cast<AirtimeFairnessGate *>(inputGate->getPathStartGate()->getOwnerModule()));
    checkPacketOperationSupport(inputGate);
    // The downstream is deliberately not notified here: the branch is still empty (and its
    // modules are not initialized yet, so it must not be looked into), and the frame whose
    // arrival created the station will notify through the branch a moment later anyway.
}

int AirtimeFairnessScheduler::schedulePacket()
{
    int n = (int)inputGates.size();
    if (n == 0)
        throw cRuntimeError("The scheduler has no input gates");
    // Deficit round robin: starting at the cursor, find the next backlogged station whose
    // gate is still eligible (open), topping up ineligible-but-backlogged stations by one
    // quantum as we pass them. Guaranteed to terminate because quantum * weight > 0 and at
    // least one station is backlogged whenever pullPacket is called.
    int emptyStreak = 0;
    while (true) {
        int index = cursor % n;
        auto gate = getGate(index);
        if (gate->isBacklogged()) {
            emptyStreak = 0;
            if (!fairnessEnabled || gate->getDeficit() >= SIMTIME_ZERO) {
                // Eligible -> serve this station. Airtime-fair keeps the cursor on it so it
                // drains its whole airtime quantum (many small frames) before yielding; the
                // async airtime charge lands before the next pull and eventually closes its
                // gate, at which point it is topped up and rotated. Frame-fair rotates now.
                //
                // Pinning the cursor is only safe while the station is actually being charged
                // for what it sends. Serving it again without its deficit having moved means
                // the previous frame was never billed -- group-addressed frames are not
                // reported by ~Dcf/~Hcf, and neither is a frame that is dequeued but never
                // transmitted -- and holding the cursor there would starve every other
                // station forever. Rotate away instead, so an unbilled station gets frame
                // fairness rather than the whole medium.
                bool charged = index != lastServedIndex || gate->getDeficit() < lastServedDeficit;
                cursor = fairnessEnabled && charged ? index : (index + 1) % n;
                lastServedIndex = index;
                lastServedDeficit = gate->getDeficit();
                return index;
            }
            else {
                // Backlogged but out of airtime credit -> grant one quantum and move on.
                gate->addQuantum();
                cursor = (index + 1) % n;
            }
        }
        else {
            cursor = (index + 1) % n;
            if (++emptyStreak >= n)
                throw cRuntimeError("No backlogged input gate available to schedule a packet");
        }
    }
}

bool AirtimeFairnessScheduler::canPullSomePacket(const cGate *gate) const
{
    // A packet is pullable whenever any station is backlogged, even if all gates are
    // momentarily closed (negative deficit): the scheduler can always top a station up.
    for (int i = 0; i < (int)inputGates.size(); i++)
        if (getGate(i)->isBacklogged())
            return true;
    return false;
}

Packet *AirtimeFairnessScheduler::canPullPacket(const cGate *gate) const
{
    for (int i = 0; i < (int)inputGates.size(); i++) {
        auto g = getGate(i);
        if (g->isBacklogged())
            return g->peekPacket();
    }
    return nullptr;
}

int AirtimeFairnessScheduler::getNumPackets() const
{
    int size = 0;
    for (auto gate : gates)
        size += gate->getNumPackets();
    return size;
}

b AirtimeFairnessScheduler::getTotalLength() const
{
    b totalLength(0);
    for (auto gate : gates)
        totalLength += gate->getTotalLength();
    return totalLength;
}

Packet *AirtimeFairnessScheduler::getPacket(int index) const
{
    int origIndex = index;
    for (auto gate : gates) {
        int numPackets = gate->getNumPackets();
        if (index < numPackets)
            return gate->getPacket(index);
        else
            index -= numPackets;
    }
    throw cRuntimeError("Index %i out of range", origIndex);
}

void AirtimeFairnessScheduler::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    for (auto gate : gates) {
        int numPackets = gate->getNumPackets();
        for (int j = 0; j < numPackets; j++) {
            if (gate->getPacket(j) == packet) {
                gate->removePacket(packet);
                return;
            }
        }
    }
    throw cRuntimeError("Cannot find packet");
}

void AirtimeFairnessScheduler::removeAllPackets()
{
    Enter_Method("removeAllPackets");
    for (auto gate : gates)
        gate->removeAllPackets();
}

} // namespace ieee80211
} // namespace inet
