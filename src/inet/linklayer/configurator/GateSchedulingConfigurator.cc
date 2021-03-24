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

#include "inet/linklayer/configurator/GateSchedulingConfigurator.h"

#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(GateSchedulingConfigurator);

void GateSchedulingConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    if (stage == INITSTAGE_QUEUEING) {
        computeConfiguration();
        configureGateScheduling();
        configureApplicationOffsets();
    }
}

void GateSchedulingConfigurator::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "configuration")) {
            configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
            clearConfiguration();
            computeConfiguration();
            configureGateScheduling();
            configureApplicationOffsets();
        }
    }
}

void GateSchedulingConfigurator::extractTopology(Topology& topology)
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

void GateSchedulingConfigurator::clearConfiguration()
{
    topology.clear();
    streamReservations.clear();
}

void GateSchedulingConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    TIME(extractTopology(topology));
    TIME(computeStreamReservations());
    TIME(computeGateScheduling());
    printElapsedTime("initialize", initializeStartTime);
}

void GateSchedulingConfigurator::computeStreamReservations()
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

void GateSchedulingConfigurator::computeGateScheduling()
{
    std::sort(streamReservations.begin(), streamReservations.end(), [] (const GateSchedulingConfigurator::StreamReservation& r1, const GateSchedulingConfigurator::StreamReservation& r2) {
        return r1.priority < r2.priority;
    });
    for (auto& streamReservation : streamReservations) {
        computeStreamStartOffset(streamReservation);
        addGateScheduling(streamReservation, 0, 1);
    }
    for (auto& streamReservation : streamReservations) {
        int count = gateCycleDuration / streamReservation.packetInterval;
        ASSERT(gateCycleDuration == count * streamReservation.packetInterval);
        addGateScheduling(streamReservation, 1, count);
    }
}

void GateSchedulingConfigurator::computeStreamStartOffset(StreamReservation& streamReservation)
{
    auto source = streamReservation.source->module;
    auto destination = streamReservation.destination->module;
    auto priority = streamReservation.priority;
    bps datarate = streamReservation.datarate;
    b packetLength = streamReservation.packetLength;
    EV_DEBUG << "Computing start offset for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(priority) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_ENDL;
    simtime_t startOffset = 0;
    while (true) {
        auto startOffsetShift = computeStartOffsetForPathFragments(streamReservation, source->getFullName(), startOffset);
        if (startOffsetShift == 0)
            break;
        else
            startOffset += startOffsetShift;
    }
    EV_DEBUG << "Setting start offset for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(priority) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_FIELD(startOffset, startOffset.ustr()) << EV_ENDL;
    streamReservation.startOffset = startOffset;
}

simtime_t GateSchedulingConfigurator::computeStartOffsetForPathFragments(StreamReservation& streamReservation, std::string startNetworkNodeName, simtime_t startTime)
{
    auto destination = streamReservation.destination->module;
    b packetLength = streamReservation.packetLength;
    simtime_t result = 0;
    std::deque<std::tuple<std::string, simtime_t, std::vector<std::string>>> todos;
    todos.push_back({startNetworkNodeName, startTime, {}});
    while (!todos.empty()) {
        auto todo = todos.front();
        todos.pop_front();
        simtime_t startOffsetShift = 0;
        for (auto& pathFragment : streamReservation.pathFragments) {
            if (pathFragment.front() == std::get<0>(todo)) {
                simtime_t nextGateOpenTime = std::get<1>(todo);
                std::vector<std::string> extendedPath = std::get<2>(todo);
                for (int i = 0; i < pathFragment.size() - 1; i++) {
                    auto networkNodeName = pathFragment[i];
                    extendedPath.push_back(networkNodeName);
                    auto networkNode = getParentModule()->getSubmodule(networkNodeName.c_str());
                    auto node = (Node *)topology.getNodeFor(networkNode);
                    auto link = findLinkOut(node, pathFragment[i + 1].c_str());
                    auto interfaceInfo = link->sourceInterfaceInfo;
                    auto networkInterface = interfaceInfo->networkInterface;
                    bps interfaceDatarate = bps(networkInterface->getDatarate());
                    simtime_t transmissionDuration = s(packetLength / interfaceDatarate).get();
                    simtime_t interFrameGap = s(b(96) / interfaceDatarate).get();
                    auto channel = dynamic_cast<cDatarateChannel *>(networkInterface->getTxTransmissionChannel());
                    simtime_t propagationDelay = channel != nullptr ? channel->getDelay() : 0;
                    auto gate = networkInterface->findModuleByPath(".macLayer.queue.gate[0]"); // KLUDGE: to check for gate scheduling
                    if (gate != nullptr) {
                        simtime_t gateOpenDuration = transmissionDuration;
                        simtime_t gateOpenTime = nextGateOpenTime;
                        simtime_t gateCloseTime = gateOpenTime + gateOpenDuration;
                        for (int i = 0; i < interfaceInfo->gateOpenIndices.size(); i++) {
                            if (interfaceInfo->gateCloseTimes[i] + interFrameGap <= gateOpenTime || gateCloseTime + interFrameGap <= interfaceInfo->gateOpenTimes[i])
                                continue;
                            else {
                                gateOpenTime = interfaceInfo->gateCloseTimes[i] + interFrameGap;
                                gateCloseTime = gateOpenTime + gateOpenDuration;
                                i = 0;
                            }
                        }
                        simtime_t gateOpenDelay = gateOpenTime - nextGateOpenTime;
                        if (gateOpenDelay != 0) {
                            startOffsetShift += gateOpenDelay;
                            break;
                        }
                        nextGateOpenTime = gateCloseTime + propagationDelay;
                    }
                    else
                        nextGateOpenTime += transmissionDuration + propagationDelay;
                }
                auto endNetworkNodeName = pathFragment.back();
                if (endNetworkNodeName != destination->getFullName() && std::find(extendedPath.begin(), extendedPath.end(), endNetworkNodeName) == extendedPath.end())
                    todos.push_back({endNetworkNodeName, nextGateOpenTime, extendedPath});
            }
        }
        result += startOffsetShift;
    }
    return result;
};

void GateSchedulingConfigurator::addGateScheduling(StreamReservation& streamReservation, int startIndex, int endIndex)
{
    auto source = streamReservation.source->module;
    auto destination = streamReservation.destination->module;
    auto priority = streamReservation.priority;
    bps datarate = streamReservation.datarate;
    b packetLength = streamReservation.packetLength;
    simtime_t startOffset = streamReservation.startOffset;
    EV_DEBUG << "Allocating gate scheduling for stream reservation" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(priority) << EV_FIELD(packetLength) << EV_FIELD(datarate) << EV_FIELD(gateCycleDuration, gateCycleDuration.ustr()) << EV_FIELD(startOffset, startOffset.ustr()) << EV_FIELD(startIndex) << EV_FIELD(endIndex) << EV_ENDL;
    for (int index = startIndex; index < endIndex; index++) {
        simtime_t startTime = startOffset + index * streamReservation.packetInterval;
        addGateSchedulingForPathFragments(streamReservation, source->getFullName(), startTime);
    }
}

void GateSchedulingConfigurator::addGateSchedulingForPathFragments(StreamReservation& streamReservation, std::string startNetworkNodeName, simtime_t startTime)
{
    auto destination = streamReservation.destination->module;
    auto priority = streamReservation.priority;
    b packetLength = streamReservation.packetLength;
    std::deque<std::tuple<std::string, simtime_t, std::vector<std::string>>> todos;
    todos.push_back({startNetworkNodeName, startTime, {}});
    while (!todos.empty()) {
        auto todo = todos.front();
        todos.pop_front();
        for (auto& pathFragment : streamReservation.pathFragments) {
            if (pathFragment.front() == std::get<0>(todo)) {
                simtime_t nextGateOpenTime = std::get<1>(todo);
                std::vector<std::string> extendedPath = std::get<2>(todo);
                for (int i = 0; i < pathFragment.size() - 1; i++) {
                    auto networkNodeName = pathFragment[i];
                    extendedPath.push_back(networkNodeName);
                    auto networkNode = getParentModule()->getSubmodule(networkNodeName.c_str());
                    auto node = (Node *)topology.getNodeFor(networkNode);
                    auto link = findLinkOut(node, pathFragment[i + 1].c_str());
                    auto interfaceInfo = link->sourceInterfaceInfo;
                    auto networkInterface = interfaceInfo->networkInterface;
                    bps interfaceDatarate = bps(networkInterface->getDatarate());
                    simtime_t transmissionDuration = s(packetLength / interfaceDatarate).get();
                    simtime_t interFrameGap = s(b(96) / interfaceDatarate).get();
                    auto channel = dynamic_cast<cDatarateChannel *>(networkInterface->getTxTransmissionChannel());
                    simtime_t propagationDelay = channel != nullptr ? channel->getDelay() : 0;
                    auto gate = networkInterface->findModuleByPath(".macLayer.queue.gate[0]"); // KLUDGE: to check for gate scheduling
                    if (gate != nullptr) {
                        int index = gate->getIndex();
                        simtime_t gateOpenDuration = transmissionDuration;
                        simtime_t gateOpenTime = nextGateOpenTime;
                        simtime_t gateCloseTime = gateOpenTime + gateOpenDuration;
                        for (int i = 0; i < interfaceInfo->gateOpenIndices.size(); i++) {
                            if (interfaceInfo->gateCloseTimes[i] + interFrameGap <= gateOpenTime || gateCloseTime + interFrameGap <= interfaceInfo->gateOpenTimes[i])
                                continue;
                            else {
                                gateOpenTime = interfaceInfo->gateCloseTimes[i] + interFrameGap;
                                gateCloseTime = gateOpenTime + gateOpenDuration;
                                i = 0;
                            }
                        }
                        simtime_t extraDelay = gateOpenTime - nextGateOpenTime;
                        EV_DEBUG << "Extending gate scheduling for stream reservation" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(priority) << EV_FIELD(interfaceDatarate) << EV_FIELD(packetLength) << EV_FIELD(index) << EV_FIELD(startTime, startTime.ustr()) << EV_FIELD(gateOpenTime, gateOpenTime.ustr()) << EV_FIELD(gateCloseTime, gateCloseTime.ustr()) << EV_FIELD(gateOpenDuration, gateOpenDuration.ustr()) << EV_FIELD(extraDelay, extraDelay.ustr()) << EV_ENDL;
                        if (gateCloseTime > gateCycleDuration)
                            throw cRuntimeError("Gate scheduling doesn't fit into cycle duration");
                        interfaceInfo->gateOpenIndices.push_back(streamReservation.priority);
                        interfaceInfo->gateOpenTimes.push_back(gateOpenTime);
                        interfaceInfo->gateCloseTimes.push_back(gateCloseTime);
                        nextGateOpenTime = gateCloseTime + propagationDelay;
                    }
                    else
                        nextGateOpenTime += transmissionDuration + propagationDelay;
                }
                auto endNetworkNodeName = pathFragment.back();
                if (endNetworkNodeName != destination->getFullName() && std::find(extendedPath.begin(), extendedPath.end(), endNetworkNodeName) == extendedPath.end())
                    todos.push_back({endNetworkNodeName, nextGateOpenTime, extendedPath});
            }
        }
    }
}

void GateSchedulingConfigurator::configureGateScheduling()
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

void GateSchedulingConfigurator::configureGateScheduling(cModule *networkNode, cModule *gate, InterfaceInfo *interfaceInfo)
{
    int index = gate->getIndex();
    cValueArray *durations = new cValueArray();
    std::stringstream stream;
    stream << "[";
    std::map<simtime_t, simtime_t> durationMap;
    for (int i = 0; i < interfaceInfo->gateOpenIndices.size(); i++)
        if (interfaceInfo->gateOpenIndices[i] == index)
            durationMap[interfaceInfo->gateOpenTimes[i]] = interfaceInfo->gateCloseTimes[i] - interfaceInfo->gateOpenTimes[i];
    bool initiallyOpen = false;
    simtime_t gateCloseTime = 0;
    for (auto & entry : durationMap) {
        if (entry.first == 0)
            initiallyOpen = true;
        else {
            simtime_t duration = entry.first - gateCloseTime;
            durations->add(cValue(duration.dbl(), "s"));
            stream << duration.ustr() << " ";
        }
        durations->add(cValue(entry.second.dbl(), "s"));
        stream << entry.second.ustr() << " ";
        gateCloseTime = entry.first + entry.second;
    }
    simtime_t remainingDuration = gateCycleDuration - gateCloseTime;
    if (gateCloseTime != 0) {
        if (remainingDuration != 0) {
            durations->add(cValue(remainingDuration.dbl(), "s"));
            stream << remainingDuration.ustr() << " ";
        }
        if (durations->size() % 2 != 0) {
            durations->add(cValue(0, "s"));
            stream << "0s ";
        }
    }
    if (durations->size() != 0)
        stream.seekp(-1, stream.cur);
    stream << "]";
    simtime_t offset = 0;
    auto networkInterface = interfaceInfo->networkInterface;
    EV_DEBUG << "Configuring gate scheduling parameters" << EV_FIELD(networkNode) << EV_FIELD(networkInterface) << EV_FIELD(gate) << EV_FIELD(initiallyOpen) << EV_FIELD(offset) << EV_FIELD(durations, stream.str()) << EV_ENDL;
    if (remainingDuration < 0)
        throw cRuntimeError("Gate scheduling doesn't fit into cycle duration");
    gate->par("initiallyOpen") = initiallyOpen;
    gate->par("offset") = offset.dbl();
    cPar& durationsPar = gate->par("durations");
    durationsPar.copyIfShared();
    durationsPar.setObjectValue(durations);
}

void GateSchedulingConfigurator::configureApplicationOffsets()
{
    for (auto& streamReservation : streamReservations) {
        if (streamReservation.startOffset != -1) {
            auto networkNode = streamReservation.source->module;
            // KLUDGE:
            auto sourceModule = networkNode->getModuleByPath(".app[0].source");
            simtime_t startOffset = streamReservation.startOffset;
            EV_DEBUG << "Setting initial packet production offset for application source" << EV_FIELD(sourceModule) << EV_FIELD(startOffset, startOffset.ustr()) << EV_ENDL;
            sourceModule->par("initialProductionOffset") = startOffset.dbl();
        }
    }
}

std::vector<std::string> GateSchedulingConfigurator::computePath(Node *source, Node *destination)
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

GateSchedulingConfigurator::Link *GateSchedulingConfigurator::findLinkOut(Node *node, const char *neighbor)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (!strcmp(node->getLinkOut(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkOut(i)));
    return nullptr;
}

Topology::LinkOut *GateSchedulingConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return nullptr;
}

GateSchedulingConfigurator::InterfaceInfo *GateSchedulingConfigurator::findInterfaceInfo(Node *node, NetworkInterface *networkInterface)
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interfaceInfo : node->interfaceInfos)
        if (interfaceInfo->networkInterface == networkInterface)
            return interfaceInfo;

    return nullptr;
}

} // namespace inet

