//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/scheduler/MarkovScheduler.h"

namespace inet {
namespace queueing {

Define_Module(MarkovScheduler);

MarkovScheduler::~MarkovScheduler()
{
    cancelAndDelete(transitionTimer);
    cancelAndDelete(waitTimer);
}

void MarkovScheduler::initialize(int stage)
{
    if (stage != INITSTAGE_QUEUEING)
        PacketSchedulerBase::initialize(stage);
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
        waitTimer = new cMessage("WaitTimer");
        WATCH(state);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (int i = 0; i < (int)inputGates.size(); i++)
            checkPacketOperationSupport(inputGates[i]);
        checkPacketOperationSupport(outputGate);
        if (producers[state] != nullptr)
            producers[state]->handleCanPushPacket(inputGates[state]->getPathStartGate());
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
            producers[state]->handleCanPushPacket(inputGates[state]->getPathStartGate());
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
    scheduleAt(simTime() + waitIntervals[state].doubleValue(this), waitTimer);
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

const char *MarkovScheduler::resolveDirective(char directive) const
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

void MarkovScheduler::handleCanPushPacket(cGate *gate)
{
    Enter_Method("handleCanPushPacket");
    if (producers[state] != nullptr)
        producers[state]->handleCanPushPacket(inputGates[state]->getPathStartGate());
}

void MarkovScheduler::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePushPacketProcessed");
    if (producers[state] != nullptr)
        producers[state]->handlePushPacketProcessed(packet, inputGates[state]->getPathStartGate(), successful);
}

} // namespace queueing
} // namespace inet

