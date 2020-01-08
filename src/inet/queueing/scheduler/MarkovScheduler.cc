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
            ActivePacketSourceRef producer;
            producer.reference(inputGates[i], false);
            producers.push_back(producer);
        }
        consumer.reference(outputGate, false);
        state = par("initialState");
        int numStates = gateSize("in");
        auto transitionProbabilitiesMatrix = check_and_cast<cValueArray*>(par("transitionProbabilities").objectValue());
        if (transitionProbabilitiesMatrix->size() != numStates)
            throw cRuntimeError("Check your transitionProbabilities parameter! Need %u rows", numStates);
        transitionProbabilities.resize(numStates);
        for (unsigned int i = 0; i < numStates; ++i) {
            transitionProbabilities[i] = check_and_cast<cValueArray*>((*transitionProbabilitiesMatrix)[i].objectValue())->asDoubleVector();
            if (transitionProbabilities[i].size() != numStates)
                throw cRuntimeError("Check your transitionProbabilities parameter! Need %u columns at row %u", numStates, i+1);
        }
        auto waitIntervalsTokens = check_and_cast<cValueArray*>(par("waitIntervals").objectValue())->asStringVector();
        if (waitIntervalsTokens.size() != numStates)
            throw cRuntimeError("Check your waitIntervals parameter! Need %u element", numStates);
        for (const auto& token : waitIntervalsTokens) {
            cDynamicExpression expression;
            expression.parse(token.c_str());
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
            producers[state].handleCanPushPacketChanged();
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
            producers[state].handleCanPushPacketChanged();
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
    scheduleClockEventAfter(waitIntervals[state].doubleValue(this, "s"), waitTimer);
}

bool MarkovScheduler::canPushSomePacket(const cGate *gate) const
{
    return gate->getIndex() == state;
}

bool MarkovScheduler::canPushPacket(Packet *packet, const cGate *gate) const
{
    return canPushSomePacket(gate);
}

void MarkovScheduler::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot push to gate");
    processedTotalLength += packet->getDataLength();
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
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

void MarkovScheduler::handleCanPushPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (producers[state] != nullptr)
        producers[state].handleCanPushPacketChanged();
}

void MarkovScheduler::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producers[state] != nullptr)
        producers[state].handlePushPacketProcessed(packet, successful);
}

} // namespace queueing
} // namespace inet

