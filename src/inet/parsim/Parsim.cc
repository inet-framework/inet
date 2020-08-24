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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/parsim/Parsim.h"

namespace inet {

Define_Module(Parsim);

void Parsim::initialize()
{
    timer = new cMessage();
    scheduleAt(simTime() + 0.01, timer);
}

void Parsim::handleMessage(cMessage *msg)
{
    auto module = getModuleByPath("routerA");
    auto gate = module->gate("pppg$i", 0);
    std::cout << "EIT: " << getEarliestInputTime(gate) << std::endl;
    scheduleAt(simTime() + 0.01, timer);
}

tf Parsim::computeMinimumChannelDelayFunction(cGate *outputGate)
{
    simtime_t delay = 0;
    auto gate = outputGate;
    while (gate != nullptr) {
        auto channel = outputGate->getChannel();
        if (channel == nullptr || dynamic_cast<cIdealChannel *>(channel))
            ;
        else if (auto delayChannel = dynamic_cast<cDelayChannel *>(channel))
            delay += delayChannel->getDelay();
        else if (auto datarateChannel = dynamic_cast<cDatarateChannel *>(channel))
            delay += datarateChannel->getDelay();
        else
            throw cRuntimeError("Unknown channel");
        gate = gate->getNextGate();
    }
    return [=] (simtime_t limit) {
        return delay;
    };
}

tf Parsim::computeEarliestModuleMethodCallFunction(cModule *module)
{
    return [] (simtime_t limit) {
        // TODO: if there are no external method calls into this module then it's simply SIMTIME_MAX
        return SIMTIME_MAX;
    };
}

tf Parsim::computeEarliestModuleEventTimeFunction(cModule *module)
{
    return [=] (simtime_t limit) {
        // TODO: if there are no events ever in the module then it's simply SIMTIME_MAX, etc.
        auto simulation = module->getSimulation();
        auto fes = simulation->getFES();
        auto l = fes->getLength();
        for (int i = 0; i < l; i++) {
            auto event = fes->get(i);
            if (event->getArrivalTime() > limit)
                return event->getArrivalTime();
            if (event->isMessage()) {
                auto message = static_cast<cMessage *>(event);
                if (message->getArrivalModule() == module)
                    return event->getArrivalTime();
            }
        }
        return SIMTIME_MAX;
    };
}

tf Parsim::computeEarliestOutputTimeFunction(cGate *outputGate)
{
    ASSERT(outputGate->getType() == cGate::OUTPUT);
    std::cout << "Building earliestOutputTimeFunction: " << outputGate->getFullPath() << std::endl;
    auto earliestModuleEventTimeFunction = computeEarliestModuleEventTimeFunction(outputGate->getOwnerModule());
    std::vector<tf> inputGateEarliestInputTimeFunctions;
    auto module = outputGate->getOwnerModule();
    for (cModule::GateIterator it(module); !it.end(); ++it) {
        cGate *g = *it;
        if (g->getType() == cGate::INPUT) {
            auto inputGateEarliestInputTimeFunction = computeEarliestInputTimeFunction(g);
            inputGateEarliestInputTimeFunctions.push_back(inputGateEarliestInputTimeFunction);
        }
    }
    auto earliestModuleMethodCallFunction = computeEarliestModuleMethodCallFunction(module);
    return [=] (simtime_t limit) {
        std::cout << "Computing earliestOutputTime: " << outputGate->getFullPath() << " " << limit << std::endl;
        simtime_t currentTime = simTime();
        simtime_t earliestOutputTime = SIMTIME_MAX;
        auto earliestModuleEventTime = earliestModuleEventTimeFunction(earliestOutputTime);
        if (earliestModuleEventTime < earliestOutputTime)
            earliestOutputTime = earliestModuleEventTime;
        auto earliestModuleMethodCallTime = earliestModuleMethodCallFunction(earliestOutputTime);
        if (earliestModuleMethodCallTime < earliestOutputTime)
            earliestOutputTime = earliestModuleMethodCallTime;
        for (auto inputGateEarliestInputTimeFunction : inputGateEarliestInputTimeFunctions) {
            if (earliestOutputTime == currentTime)
                break;
            simtime_t inputGateEarliestInputTime = inputGateEarliestInputTimeFunction(earliestOutputTime);
            if (inputGateEarliestInputTime < earliestOutputTime)
                earliestOutputTime = inputGateEarliestInputTime;
        }
        return earliestOutputTime;
    };
}

tf Parsim::computeEarliestInputTimeFunction(cGate *inputGate)
{
    ASSERT(inputGate->getType() == cGate::INPUT);
    std::cout << "Building earliestInputTimeFunction: " << inputGate->getFullPath() << std::endl;
    // TODO: handle direct sends
    auto outputGate = inputGate->getPathStartGate();
    if (outputGate->getType() == cGate::OUTPUT) {
        tf outputGateEarliestOutputTimeFunction = nullptr;
        tf minimumChannelDelayFunction = nullptr;
        return [=] (simtime_t limit) mutable {
            std::cout << "Computing earliestInputTime: " << inputGate->getFullPath() << " " << limit << std::endl;
            if (outputGateEarliestOutputTimeFunction == nullptr)
                outputGateEarliestOutputTimeFunction = computeEarliestOutputTimeFunction(outputGate);
            if (minimumChannelDelayFunction == nullptr)
                minimumChannelDelayFunction = computeMinimumChannelDelayFunction(outputGate);
            simtime_t minimumChannelDelay = minimumChannelDelayFunction(limit);
            if (minimumChannelDelay > limit)
                return minimumChannelDelay;
            return outputGateEarliestOutputTimeFunction(limit) + minimumChannelDelay;
        };
    }
    else
        return [] (simtime_t limit) {
            return SIMTIME_MAX;
        };
}

simtime_t Parsim::getEarliestInputTime(cGate *inputGate)
{
    auto earliestInputTimeFunction = computeEarliestInputTimeFunction(inputGate);
    return earliestInputTimeFunction(SIMTIME_MAX);
}

} // namespace

