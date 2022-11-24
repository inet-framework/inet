//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/classifier/MarkovClassifier.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(MarkovClassifier);

MarkovClassifier::~MarkovClassifier()
{
    cancelAndDeleteClockEvent(waitTimer);
}

void MarkovClassifier::initialize(int stage)
{
    if (stage != INITSTAGE_QUEUEING)
        ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("out"); i++) {
            auto output = findConnectedModule<IActivePacketSink>(outputGates[i]);
            collectors.push_back(output);
        }
        provider = findConnectedModule<IPassivePacketSource>(inputGate);
        state = par("initialState");
        int numStates = gateSize("out");
        cStringTokenizer transitionProbabilitiesTokenizer(par("transitionProbabilities"));
        for (int i = 0; i < numStates; i++) {
            transitionProbabilities.push_back({});
            for (int j = 0; j < numStates; j++)
                transitionProbabilities[i].push_back(atof(transitionProbabilitiesTokenizer.nextToken()));
        }
        cStringTokenizer waitIntervalsTokenizer(par("waitIntervals"));
        for (int i = 0; i < numStates; i++) {
            cDynamicExpression expression;
            expression.parse(waitIntervalsTokenizer.nextToken());
            waitIntervals.push_back(expression);
        }
        waitTimer = new ClockEvent("WaitTimer");
        WATCH(state);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (auto& outputGate : outputGates)
            checkPacketOperationSupport(outputGate);
        checkPacketOperationSupport(inputGate);
        if (collectors[state] != nullptr)
            collectors[state]->handleCanPullPacketChanged(outputGates[state]->getPathEndGate());
        scheduleWaitTimer();
    }
}

void MarkovClassifier::handleMessage(cMessage *message)
{
    if (message == waitTimer) {
        double v = uniform(0, 1);
        double sum = 0;
        int numStates = (int)transitionProbabilities.size();
        for (int i = 0; i < numStates; i++) {
            sum += transitionProbabilities[state][i];
            if (sum >= v || i == numStates - 1) {
                state = i;
                break;
            }
        }
        if (collectors[state] != nullptr)
            collectors[state]->handleCanPullPacketChanged(outputGates[state]->getPathEndGate());
        scheduleWaitTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

int MarkovClassifier::classifyPacket(Packet *packet)
{
    return state;
}

void MarkovClassifier::scheduleWaitTimer()
{
    scheduleClockEventAfter(waitIntervals[state].doubleValue(this), waitTimer);
}

bool MarkovClassifier::canPullSomePacket(cGate *gate) const
{
    return gate->getIndex() == state;
}

Packet *MarkovClassifier::canPullPacket(cGate *gate) const
{
    return canPullSomePacket(gate) ? provider->canPullPacket(inputGate->getPathStartGate()) : nullptr;
}

Packet *MarkovClassifier::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot pull from gate");
    auto packet = provider->pullPacket(inputGate->getPathStartGate());
    take(packet);
    animatePullPacket(packet, gate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

std::string MarkovClassifier::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return std::to_string(state);
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

void MarkovClassifier::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collectors[state] != nullptr)
        collectors[state]->handleCanPullPacketChanged(outputGates[state]->getPathEndGate());
}

void MarkovClassifier::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collectors[state] != nullptr)
        collectors[state]->handlePullPacketProcessed(packet, outputGates[state]->getPathEndGate(), successful);
}

} // namespace queueing
} // namespace inet

