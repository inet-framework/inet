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
            ActivePacketSinkRef collector;
            collector.reference(outputGates[i], false);
            collectors.push_back(collector);
        }
        provider.reference(inputGate, false);
        state = par("initialState");
        unsigned int numStates = gateSize("out");
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
        for (auto& outputGate : outputGates)
            checkPacketOperationSupport(outputGate);
        checkPacketOperationSupport(inputGate);
        if (collectors[state] != nullptr)
            collectors[state].handleCanPullPacketChanged();
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
            collectors[state].handleCanPullPacketChanged();
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
    scheduleClockEventAfter(waitIntervals[state].doubleValue(this, "s"), waitTimer);
}

bool MarkovClassifier::canPullSomePacket(const cGate *gate) const
{
    return gate->getIndex() == state;
}

Packet *MarkovClassifier::canPullPacket(const cGate *gate) const
{
    return canPullSomePacket(gate) ? provider.canPullPacket() : nullptr;
}

Packet *MarkovClassifier::pullPacket(const cGate *gate)
{
    Enter_Method("pullPacket");
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot pull from gate");
    auto packet = provider.pullPacket();
    take(packet);
    animatePullPacket(packet, outputGates[gate->getIndex()], findConnectedGate<IActivePacketSink>(gate));
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
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

void MarkovClassifier::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (collectors[state] != nullptr)
        collectors[state].handleCanPullPacketChanged();
}

void MarkovClassifier::handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    if (collectors[state] != nullptr)
        collectors[state].handlePullPacketProcessed(packet, successful);
}

} // namespace queueing
} // namespace inet

