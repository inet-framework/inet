//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/linklayer/configurator/gatescheduling/z3/Z3GateSchedulingConfigurator.h"

#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

Define_Module(Z3GateSchedulingConfigurator);

void Z3GateSchedulingConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    else if (stage == INITSTAGE_QUEUEING) {
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void Z3GateSchedulingConfigurator::extractTopology(Topology& topology)
{
    topology.extractByProperty("networkNode");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    if (topology.getNumNodes() == 0)
        throw cRuntimeError("Empty network!");

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        node->module = node->getModule();
        node->interfaceTable = dynamic_cast<IInterfaceTable *>(node->module->getSubmodule("interfaceTable"));
        node->routingTable = L3AddressResolver().findIpv4RoutingTableOf(node->module);
    }

    // extract links and interfaces
    std::set<NetworkInterface *> interfacesSeen;
    std::queue<Node *> unvisited; // unvisited nodes in the graph
    auto rootNode = (Node *)topology.getNode(0);
    unvisited.push(rootNode);
    while (!unvisited.empty()) {
        Node *node = unvisited.front();
        unvisited.pop();
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable) {
            // push neighbors to the queue
            for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                NetworkInterface *networkInterface = interfaceTable->getInterface(i);
                if (interfacesSeen.count(networkInterface) == 0) {
                    // visiting this interface
                    interfacesSeen.insert(networkInterface);
                    Topology::LinkOut *linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
                    Node *childNode = nullptr;
                    if (linkOut) {
                        childNode = (Node *)linkOut->getRemoteNode();
                        unvisited.push(childNode);
                    }
                    InterfaceInfo *info = new InterfaceInfo(networkInterface);
                    node->interfaceInfos.push_back(info);
                }
            }
        }
    }
    // annotate links with interfaces
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::LinkOut *linkOut = node->getLinkOut(j);
            Link *link = (Link *)linkOut;
            Node *localNode = (Node *)linkOut->getLocalNode();
            if (localNode->interfaceTable)
                link->sourceInterfaceInfo = findInterfaceInfo(localNode, localNode->interfaceTable->findInterfaceByNodeOutputGateId(linkOut->getLocalGateId()));
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            if (remoteNode->interfaceTable)
                link->destinationInterfaceInfo = findInterfaceInfo(remoteNode, remoteNode->interfaceTable->findInterfaceByNodeInputGateId(linkOut->getRemoteGateId()));
        }
    }
}

void Z3GateSchedulingConfigurator::clearConfiguration()
{
    topology.clear();
}

void Z3GateSchedulingConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    TIME(extractTopology(topology));
    TIME(computeStreamReservations());
    TIME(computeGateScheduling());
    printElapsedTime("initialize", initializeStartTime);
}

void Z3GateSchedulingConfigurator::computeStreamReservations()
{
    EV_DEBUG << "Computing stream reservations according to configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto sourceNode = (Node *)topology.getNode(i);
        cModule *source = sourceNode->module;
        for (int j = 0; j < topology.getNumNodes(); j++) {
            auto destinationNode = (Node *)topology.getNode(j);
            cModule *destination = destinationNode->module;
            for (int k = 0; k < configuration->size(); k++) {
                auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
                PatternMatcher sourceMatcher(entry->get("source").stringValue(), true, false, false);
                PatternMatcher destinationMatcher(entry->get("destination").stringValue(), true, false, false);
                if (sourceMatcher.matches(sourceNode->module->getFullPath().c_str()) &&
                    destinationMatcher.matches(destinationNode->module->getFullPath().c_str()))
                {
                    int priority = entry->get("priority").intValue();
                    b packetLength = b(entry->get("packetLength").doubleValueInUnit("b"));
                    simtime_t packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
                    simtime_t maxLatency = entry->get("maxLatency").doubleValueInUnit("s");
                    bps datarate = packetLength / s(packetInterval.dbl());
                    EV_DEBUG << "Adding stream reservation from configuration" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(priority) << EV_FIELD(packetLength) << EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(datarate) << EV_FIELD(maxLatency, maxLatency.ustr()) << EV_ENDL;
                    StreamReservation streamReservation;
                    streamReservation.source = sourceNode;
                    streamReservation.destination = destinationNode;
                    streamReservation.priority = priority;
                    streamReservation.packetLength = packetLength;
                    streamReservation.packetInterval = packetInterval;
                    streamReservation.maxLatency = maxLatency;
                    streamReservation.datarate = datarate;
                    if (entry->containsKey("pathFragments")) {
                        auto pathFragments = check_and_cast<cValueArray *>(entry->get("pathFragments").objectValue());
                        for (int l = 0; l < pathFragments->size(); l++) {
                            std::vector<std::string> path;
                            auto pathFragment = check_and_cast<cValueArray *>(pathFragments->get(l).objectValue());
                            for (int m = 0; m < pathFragment->size(); m++)
                                path.push_back(pathFragment->get(m).stringValue());
                            streamReservation.pathFragments.push_back(path);
                        }
                    }
                    else
                        streamReservation.pathFragments.push_back(computePath(sourceNode, destinationNode));
                    streamReservations.push_back(streamReservation);
                }
            }
        }
    }
}

void Z3GateSchedulingConfigurator::computeGateScheduling()
{
    Network *network = new Network();
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        if (isBridgeNode(node)) {
            double cycleDurationUpperBound = (gateCycleDuration * 1000000).dbl();
            Switch *tsnSwitch = new Switch(networkNode->getFullName(), 0, cycleDurationUpperBound);
            node->device = tsnSwitch;
            network->addSwitch(tsnSwitch);
        }
        else {
            Device *device = new Device(0, 0, 0, 0);
            device->setName(networkNode->getFullName());
            node->device = device;
        }
    }
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        if (isBridgeNode(node)) {
            for (int j = 0; j < node->getNumOutLinks(); j++) {
                auto link = node->getLinkOut(j);
                Node *localNode = (Node *)link->getLocalNode();
                Node *remoteNode = (Node *)link->getRemoteNode();
                Cycle *cycle = new Cycle((gateCycleDuration * 1000000).dbl(), 0, (gateCycleDuration * 1000000).dbl());
                ((Link *)link)->sourceInterfaceInfo->cycle = cycle;
                // TODO datarate, propagation time
                double datarate = 1E+9 / 1000000;
                double propagationTime = 50E-9 * 1000000;
                double guardBand = 0;
                for (auto& streamReservation : streamReservations) {
                    double v = b(streamReservation.packetLength).get() / datarate;
                    if (guardBand < v)
                        guardBand = v;
                }
                check_and_cast<Switch *>(localNode->device)->createPort(remoteNode->device, cycle, 1500 * 8, propagationTime, datarate, guardBand);
            }
        }
    }
    for (auto& streamReservation : streamReservations) {
        Flow *flow = new Flow(Flow::UNICAST);
        flow->setFixedPriority(true);
        flow->setPriorityValue(streamReservation.priority);
        topology.calculateUnweightedSingleShortestPathsTo(streamReservation.destination);
        auto startDevice = check_and_cast<Device *>(streamReservation.source->device);
        startDevice->setPacketPeriodicity((streamReservation.packetInterval * 1000000).dbl());
        startDevice->setPacketSize(streamReservation.packetLength.get() + 12 * 8);
        startDevice->setHardConstraintTime((streamReservation.maxLatency * 1000000).dbl());
        flow->setStartDevice(startDevice);
        auto node = streamReservation.source;
        node = (Node *)node->getPath(0)->getRemoteNode();
        while (node != streamReservation.destination) {
            flow->addToPath(check_and_cast<Switch *>(node->device));
            node = (Node *)node->getPath(0)->getRemoteNode();
        }
        flow->setEndDevice(check_and_cast<Device *>(streamReservation.destination->device));
        network->addFlow(flow);
        streamReservation.flow = flow;
    }
    generateSchedule(network);
}

void Z3GateSchedulingConfigurator::configureGateScheduling()
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        for (auto& elem : node->interfaceInfos) {
            auto interfaceInfo = static_cast<InterfaceInfo *>(elem);
            auto queue = interfaceInfo->networkInterface->findModuleByPath(".macLayer.queue");
            if (queue != nullptr) {
                for (cModule::SubmoduleIterator it(queue); !it.end(); ++it) {
                    cModule *gate = *it;
                    if (!strcmp(gate->getName(), "gate"))
                        configureGateScheduling(networkNode, gate, interfaceInfo);
                }
            }
        }
    }
}

void Z3GateSchedulingConfigurator::configureGateScheduling(cModule *networkNode, cModule *gate, InterfaceInfo *interfaceInfo)
{
    int priority = gate->getIndex();
    Cycle *cycle = interfaceInfo->cycle;
    auto cycleStart = us(cycle->getCycleStart());
    auto cycleDuration = us(cycle->getCycleDuration());
    auto networkInterface = interfaceInfo->networkInterface;
    EV_DEBUG << "Configuring cycle" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(priority) << EV_FIELD(cycleDuration) << EV_FIELD(cycleStart) << EV_ENDL;
    std::vector<std::pair<simtime_t, simtime_t>> startsAndDurations;
    for (int i = 0; i < cycle->getSlotsUsed().size(); i++) {
        int currentQ = cycle->getSlotsUsed().at(i);
        if (priority != currentQ)
            continue;
//        //convert queue ID to binary encoded value
//        int binCurrentQ = (int) Math.pow(2,currentQ);
        // find all slot durations for Q i
        // second loop, because there could be more slots than priorities
        for (int j = 0; j < cycle->getSlotDuration().at(i).size(); j++) {
            auto slotStart = us(cycle->getSlotStart(currentQ, j));
            auto slotDuration = us(cycle->getSlotDuration(currentQ, j));
            // slot with length 0 are not used
            if (slotDuration == s(0))
                continue;
            EV_DEBUG << "Configuring slot" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(priority) << EV_FIELD(slotStart) << EV_FIELD(slotDuration) << EV_ENDL;
            startsAndDurations.push_back({s(slotStart).get(), s(slotDuration).get()});
            // check if slot duration is smaller than an unsigned 32 bit integer
            // (type provided by TrustNode yang model) roughly 4,29 seconds
//            if (slotDuration <= MAX_UINT32) {
//                long slotStart = (long) cycle->getSlotStart(currentQ, j);
//
//                // sort all values into a list of triples
//                allSlots.add(new Triple(binCurrentQ, slotStart, slotDuration));
//            }
//            else {
//                throw new NumberFormatException("Cycle duration too large.");
//            }
        }
    }
    simtime_t offset = 0;
    bool initiallyOpen = true; // TODO ?
    auto durations = computeDurations(startsAndDurations, initiallyOpen);
    EV_DEBUG << "Configuring gate scheduling parameters" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(gate) << EV_FIELD(initiallyOpen) << EV_FIELD(offset) << EV_FIELD(durations) << EV_ENDL;
    gate->par("initiallyOpen") = initiallyOpen;
    gate->par("offset") = offset.dbl(); // TODO ? cycleStart.get();
    cPar& durationsPar = gate->par("durations");
    durationsPar.copyIfShared();
    durationsPar.setObjectValue(durations);
}

cValueArray *Z3GateSchedulingConfigurator::computeDurations(std::vector<std::pair<simtime_t, simtime_t>>& startsAndDurations, bool& initiallyOpen)
{
    cValueArray *durations = new cValueArray();
    std::map<simtime_t, simtime_t> durationMap;
    initiallyOpen = false;
    simtime_t endTime = 0;
    for (auto & entry : startsAndDurations) {
        if (entry.first == 0)
            initiallyOpen = true;
        else {
            simtime_t duration = entry.first - endTime;
            if (duration < 0)
                duration = 0; // KLUDGE TODO to avoid numeric accuracy errors
            durations->add(cValue(duration.dbl(), "s"));
        }
        durations->add(cValue(entry.second.dbl(), "s"));
        endTime = entry.first + entry.second;
    }
    simtime_t remainingDuration = gateCycleDuration - endTime;
    if (endTime != 0) {
        if (remainingDuration != 0)
            durations->add(cValue(remainingDuration.dbl(), "s"));
        if (durations->size() % 2 != 0)
            durations->add(cValue(0, "s"));
    }
    return durations;
}

void Z3GateSchedulingConfigurator::configureApplicationOffsets()
{
    for (auto& streamReservation : streamReservations) {
        // TODO datarate
        double datarate = 1E+9;
        auto startOffset = us(streamReservation.flow->getFlowFirstSendingTime()) - s((streamReservation.packetLength.get() + 12 * 8) / datarate);
        // KLUDGE TODO workaround a numerical accuracy problem that this number comes out negative sometimes
        if (startOffset < s(0))
            startOffset = s(0);
        ASSERT(startOffset >= s(0));
        auto networkNode = streamReservation.source->module;
        // KLUDGE:
        auto sourceModule = networkNode->getModuleByPath(".app[0].source");
        EV_DEBUG << "Setting initial packet production offset for application source" << EV_FIELD(sourceModule) << EV_FIELD(startOffset) << EV_ENDL;
        sourceModule->par("initialProductionOffset") = s(startOffset).get();
    }
}

std::vector<std::string> Z3GateSchedulingConfigurator::computePath(Node *source, Node *destination)
{
    std::vector<std::string> path;
    topology.calculateUnweightedSingleShortestPathsTo(destination);
    auto node = source;
    while (node != destination) {
        auto networkNode = node->module;
        path.push_back(networkNode->getFullName());
        node = (Node *)node->getPath(0)->getRemoteNode();
    }
    path.push_back(destination->module->getFullName());
    return path;
}

Z3GateSchedulingConfigurator::Link *Z3GateSchedulingConfigurator::findLinkOut(Node *node, const char *neighbor)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (!strcmp(node->getLinkOut(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkOut(i)));
    return nullptr;
}

Topology::LinkOut *Z3GateSchedulingConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return nullptr;
}

Z3GateSchedulingConfigurator::InterfaceInfo *Z3GateSchedulingConfigurator::findInterfaceInfo(Node *node, NetworkInterface *networkInterface)
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interfaceInfo : node->interfaceInfos)
        if (interfaceInfo->networkInterface == networkInterface)
            return interfaceInfo;

    return nullptr;
}

bool Z3GateSchedulingConfigurator::isBridgeNode(Node *node)
{
    return !node->routingTable || !node->interfaceTable;
}

} // namespace inet

