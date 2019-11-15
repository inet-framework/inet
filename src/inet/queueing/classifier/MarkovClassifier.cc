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
#include "inet/queueing/classifier/MarkovClassifier.h"

namespace inet {
namespace queueing {

Define_Module(MarkovClassifier);

MarkovClassifier::~MarkovClassifier()
{
    cancelAndDelete(transitionTimer);
    cancelAndDelete(waitTimer);
}

void MarkovClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("out"); i++) {
            auto output = getConnectedModule<IActivePacketSink>(outputGates[i]);
            collectors.push_back(output);
        }
        provider = getConnectedModule<IPassivePacketSource>(inputGate);
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
        waitTimer = new cMessage("WaitTimer");
        WATCH(state);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (auto outputGate : outputGates)
            checkPopPacketSupport(outputGate);
        checkPopPacketSupport(inputGate);
        collectors[state]->handleCanPopPacket(outputGates[state]);
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
        collectors[state]->handleCanPopPacket(outputGates[state]);
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
    scheduleAt(simTime() + waitIntervals[state].doubleValue(this), waitTimer);
}

bool MarkovClassifier::canPopSomePacket(cGate *gate)
{
    return gate->getIndex() == state;
}

Packet *MarkovClassifier::canPopPacket(cGate *gate)
{
    return canPopSomePacket(gate) ? provider->canPopPacket(inputGate->getPathStartGate()) : nullptr;
}

Packet *MarkovClassifier::popPacket(cGate *gate)
{
    Enter_Method("popPacket");
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot pop from gate");
    auto packet = provider->popPacket(inputGate->getPathEndGate());
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    updateDisplayString();
    animateSend(packet, gate);
    return packet;
}

void MarkovClassifier::handleCanPopPacket(cGate *gate)
{
    Enter_Method("handleCanPopPacket");
    collectors[state]->handleCanPopPacket(outputGates[state]);
}

} // namespace queueing
} // namespace inet

