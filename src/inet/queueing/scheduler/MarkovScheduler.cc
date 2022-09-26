//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/scheduler/MarkovScheduler.h"

#include "inet/common/ModuleAccess.h"

namespace inet {
namespace queueing {

Define_Module(MarkovScheduler);

MarkovScheduler::~MarkovScheduler()
{
    cancelAndDeleteClockEvent(waitTimer);
}

void MarkovScheduler::initialize(int stage)
{
    if (stage != INITSTAGE_QUEUEING)
        ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("in"); i++) {
            auto input = findConnectedModule<IActivePacketSource>(inputGates[i]);
            producers.push_back(input);
        }
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
        state = par("initialState");
        int numStates = gateSize("in");
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
        for (auto& inputGate : inputGates)
            checkPacketOperationSupport(inputGate);
        checkPacketOperationSupport(outputGate);
        if (producers[state] != nullptr)
            producers[state]->handleCanPushPacketChanged(inputGates[state]->getPathStartGate());
        scheduleWaitTimer();
    }
}

void MarkovScheduler::handleMessage(cMessage *message)
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
        if (producers[state] != nullptr)
            producers[state]->handleCanPushPacketChanged(inputGates[state]->getPathStartGate());
        scheduleWaitTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

int MarkovScheduler::schedulePacket()
{
    return state;
}

void MarkovScheduler::scheduleWaitTimer()
{
    scheduleClockEventAfter(waitIntervals[state].doubleValue(this), waitTimer);
}

bool MarkovScheduler::canPushSomePacket(cGate *gate) const
{
    return gate->getIndex() == state;
}

bool MarkovScheduler::canPushPacket(Packet *packet, cGate *gate) const
{
    return canPushSomePacket(gate);
}

void MarkovScheduler::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot push to gate");
    processedTotalLength += packet->getDataLength();
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    updateDisplayString();
}

std::string MarkovScheduler::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return std::to_string(state);
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
}

void MarkovScheduler::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producers[state] != nullptr)
        producers[state]->handleCanPushPacketChanged(inputGates[state]->getPathStartGate());
}

void MarkovScheduler::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producers[state] != nullptr)
        producers[state]->handlePushPacketProcessed(packet, inputGates[state]->getPathStartGate(), successful);
}

} // namespace queueing
} // namespace inet

