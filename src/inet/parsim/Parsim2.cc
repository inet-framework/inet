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

#include "inet/common/ModuleAccess.h"
#include "inet/parsim/Parsim2.h"

namespace inet {

Define_Module(Parsim2);

void Parsim2::initialize()
{
    timer = new cMessage();
    scheduleAt(simTime() + 0.001, timer);
}

void Parsim2::handleMessage(cMessage *msg)
{
    collectEntries();
    optimizeEntries();
    verifyOptimizedEntries();
//    printOptimizedEntries();
    calculateEarliestInputTimes();
    scheduleAt(simTime() + 0.001, timer);
}

void Parsim2::collectEntries()
{
    auto simulation = getSimulation();
    cTopology topology;
    for (int i = 0; i < simulation->getLastComponentId(); i++) {
        if (auto module = dynamic_cast<cModule *>(simulation->getComponent(i))) {
            auto sourceNode = new cTopology::Node(module->getId());
            topology.addNode(sourceNode);
        }
    }
    for (int i = 0; i < simulation->getLastComponentId(); i++) {
        if (auto sourceModule = dynamic_cast<cModule *>(simulation->getComponent(i))) {
            auto sourceNode = topology.getNodeFor(sourceModule);
            for (cModule::GateIterator it(sourceModule); !it.end(); ++it) {
                cGate *gate = *it;
                if (gate->getType() == cGate::OUTPUT) {
                    auto destinationModule = check_and_cast<cModule *>(gate->getPathEndGate()->getOwner());
                    auto destinationNode = topology.getNodeFor(destinationModule);
                    if (destinationNode != nullptr) {
                        simtime_t delay = 0;
                        while (gate != nullptr) {
                            auto channel = gate->getChannel();
                            if (channel == nullptr || dynamic_cast<cIdealChannel *>(channel))
                                ;
                            else if (auto delayChannel = dynamic_cast<cDelayChannel *>(channel))
                                delay += delayChannel->getDelay();
                            else if (auto datarateChannel = dynamic_cast<cDatarateChannel *>(channel)) {
                                auto d = datarateChannel->getDelay();
                                if (datarateChannel->isBusy()) {
                                    auto d1 = datarateChannel->getTransmissionFinishTime() - simTime();
                                    if (d1 > d)
                                        d = d1;
                                }
                                delay += d;
                            }
                            else
                                throw cRuntimeError("Unknown channel");
                            gate = gate->getNextGate();
                        }
                        auto link = new cTopology::Link(delay.dbl());
                        topology.addLink(link, sourceNode, destinationNode);
                    }
                }
            }
        }
    }
    entries.clear();
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto sourceNode = topology.getNode(i);
        topology.calculateWeightedSingleShortestPathsTo(sourceNode);
        for (int j = 0; j < topology.getNumNodes(); j++) {
            auto destinationNode = topology.getNode(j);
            simtime_t delay = 0;
            auto node = destinationNode;
            while (node != sourceNode) {
                auto link = node->getPath(0);
                if (link == nullptr) {
                    delay = -1;
                    break;
                }
                else {
                    delay += link->getWeight();
                    node = node->getPath(0)->getRemoteNode();
                }
            }
            if (delay != -1)
                entries.push_back(new ModuleEntry(sourceNode->getModule(), destinationNode->getModule(), delay));
        }
    }
}

void Parsim2::optimizeEntries()
{
    for (auto entry : entries) {
        auto sourceNetworkNode = findContainingNode(entry->source);
        auto destinationNetworkNode = findContainingNode(entry->destination);
        if (sourceNetworkNode != nullptr && destinationNetworkNode != nullptr) {
            for (auto optimizedEntry : optimizedEntries) {
                if (optimizedEntry->source == sourceNetworkNode && optimizedEntry->destination == destinationNetworkNode)
                    goto next;
            }
            optimizedEntries.push_back(new ModuleTreeEntry(sourceNetworkNode, destinationNetworkNode, entry->delay));
        }
        next:;
    }
}

void Parsim2::verifyOptimizedEntries()
{
    for (auto entry : entries) {
        auto sourceNetworkNode = findContainingNode(entry->source);
        auto destinationNetworkNode = findContainingNode(entry->destination);
        if (sourceNetworkNode != nullptr && destinationNetworkNode != nullptr) {
            for (auto optimizedEntry : optimizedEntries) {
                if (optimizedEntry->source == sourceNetworkNode && optimizedEntry->destination == destinationNetworkNode && optimizedEntry->delay == entry->delay)
                    goto next;
            }
//            ASSERT(false);
        }
        next:;
    }
}

void Parsim2::printEntries()
{
    for (auto entry : entries) {
        std::cout << "Entry: " << entry->source->getFullPath() << " -> " << entry->destination->getFullPath() << " : " << entry->delay << std::endl;
    }
}

void Parsim2::printOptimizedEntries()
{
    for (auto entry : optimizedEntries) {
        std::cout << "Entry: " << entry->source->getFullPath() << " -> " << entry->destination->getFullPath() << " : " << entry->delay << std::endl;
    }
}

void Parsim2::calculateEarliestInputTimes()
{
    auto fes = getSimulation()->getFES();
    std::vector<simtime_t> earliestInputTimes;
    for (int i = 0; i < fes->getLength(); i++)
        earliestInputTimes.push_back(SIMTIME_MAX);
    for (int i = 0; i < fes->getLength(); i++) {
        auto sourceEvent = fes->get(i);
        if (sourceEvent->isMessage()) {
            auto sourceMessage = static_cast<cMessage *>(sourceEvent);
            auto sourceModule = sourceMessage->getArrivalModule();
            auto sourceNetworkNode = findContainingNode(sourceModule);
            for (auto entry : optimizedEntries) {
                if (entry->source == entry->destination)
                    continue;
                if (entry->source == sourceNetworkNode) {
                    auto earliestInputTime = sourceMessage->getArrivalTime() + entry->delay;
                    for (int j = 0; j < fes->getLength(); j++) {
                        auto destinationEvent = fes->get(j);
                        if (destinationEvent->isMessage()) {
                            auto destinationMessage = static_cast<cMessage *>(destinationEvent);
                            auto destinationModule = destinationMessage->getArrivalModule();
                            auto destinationNetworkNode = findContainingNode(destinationModule);
                            if (entry->destination == destinationNetworkNode) {
                                if (earliestInputTime < earliestInputTimes[j])
                                    earliestInputTimes[j] = earliestInputTime;
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "Concurrent events" << std::endl;
    for (int i = 0; i < fes->getLength(); i++) {
        auto event = fes->get(i);
        if (event->isMessage()) {
            auto message = static_cast<cMessage *>(event);
            auto module = message->getArrivalModule();
            if (earliestInputTimes[i] > message->getArrivalTime())
                std::cout << "Next event, module = " << module->getFullPath() << ", time = " << message->getArrivalTime() << ", eit = " << earliestInputTimes[i] << std::endl;
        }
    }
}

} // namespace

