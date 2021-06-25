//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    scheduleClockEventAfter(waitIntervals[state].doubleValue(this, "s"), waitTimer);
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
    auto packet = provider->pullPacket(inputGate->getPathEndGate());
    take(packet);
    animatePullPacket(packet, gate);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    return packet;
}

const char *MarkovClassifier::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 's':
            result = std::to_string(state);
            break;
        default:
            return PacketProcessorBase::resolveDirective(directive);
    }
    return result.c_str();
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

