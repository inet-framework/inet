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

#include "inet/linklayer/configurator/TsnConfigurator.h"

#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(TsnConfigurator);

void TsnConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureStreams();
    }
}

void TsnConfigurator::extractTopology(Topology& topology)
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

void TsnConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    TIME(extractTopology(topology));
    TIME(computeStreams());
    printElapsedTime("initialize", initializeStartTime);
}

void TsnConfigurator::computeStreams()
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        computeStream(streamConfiguration);
    }
}

void TsnConfigurator::computeStream(cValueMap *configuration)
{
    StreamConfiguration streamConfiguration;
    streamConfiguration.name = configuration->get("name").stringValue();
    streamConfiguration.packetFilter = configuration->get("packetFilter").stringValue();
    auto sourceNetworkNodeName = configuration->get("source").stringValue();
    auto destinationNetworkNodeName = configuration->get("destination").stringValue();
    streamConfiguration.source = sourceNetworkNodeName;
    streamConfiguration.destination = destinationNetworkNodeName;
    Node *source = static_cast<Node *>(topology.getNodeFor(getParentModule()->getSubmodule(sourceNetworkNodeName)));
    Node *destination = static_cast<Node *>(topology.getNodeFor(getParentModule()->getSubmodule(destinationNetworkNodeName)));
    topology.calculateUnweightedSingleShortestPathsTo(source);
    std::vector<std::vector<std::string>> allPaths;
    collectAllPaths(source, destination, allPaths);
    streamConfiguration.paths = selectBestPathsSubset(configuration, allPaths);
    streamConfigurations.push_back(streamConfiguration);
}

std::vector<std::vector<std::string>> TsnConfigurator::selectBestPathsSubset(cValueMap *configuration, const std::vector<std::vector<std::string>>& paths)
{
    cValueArray *nodeFailureProtection = configuration->containsKey("nodeFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("nodeFailureProtection").objectValue()) : nullptr;
    cValueArray *linkFailureProtection = configuration->containsKey("linkFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("linkFailureProtection").objectValue()) : nullptr;
    int n = paths.size();
    for (int k = 1; k <= paths.size(); k++) {
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        std::vector<bool> bestMask;
        double bestCost = DBL_MAX;
        do {
            double cost = 0;
            for (int i = 0; i < n; i++)
                if (mask[i])
                    cost += paths[i].size();
            cost /= k;
            if (cost < bestCost) {
                std::vector<std::vector<std::string>> candidate;
                for (int i = 0; i < n; i++)
                    if (mask[i])
                        candidate.push_back(paths[i]);
                if ((nodeFailureProtection == nullptr || checkNodeFailureProtection(nodeFailureProtection, candidate)) &&
                    (linkFailureProtection == nullptr || checkLinkFailureProtection(linkFailureProtection, candidate)))
                {
                    bestCost = cost;
                    bestMask = mask;
                }
            }
        } while (std::prev_permutation(mask.begin(), mask.end()));
        if (bestCost != DBL_MAX) {
            std::vector<std::vector<std::string>> result;
            for (int i = 0; i < n; i++)
                if (bestMask[i])
                    result.push_back(paths[i]);
            return result;
        }
    }
    throw cRuntimeError("Cannot find path combination that protects against all configured node and link failures");
}

bool TsnConfigurator::checkNodeFailureProtection(cValueArray *configuration, const std::vector<std::vector<std::string>>& paths)
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkNodes = collectNetworkNodes(of);
        int n = networkNodes.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        do {
            std::vector<std::string> failed;
            for (int i = 0; i < n; i++)
                if (mask[i])
                    failed.push_back(networkNodes[i]);
            bool isProtected = false;
            for (int i = 0; i < paths.size(); i++) {
                if (!intersects(paths[i], failed)) {
                    isProtected = true;
                    break;
                }
            }
            if (!isProtected)
                return false;
        } while (std::prev_permutation(mask.begin(), mask.end()));
    }
    return true;
}

bool TsnConfigurator::checkLinkFailureProtection(cValueArray *configuration, const std::vector<std::vector<std::string>>& paths)
{
    std::vector<std::vector<std::string>> linkPaths;
    for (auto path : paths) {
        std::vector<std::string> linkPath;
        std::string previousNode;
        for (int i = 0; i < path.size(); i++) {
            auto node = path[i];
            if (!previousNode.empty()) {
                auto link = previousNode + "->" + node;
                linkPath.push_back(link);
            }
            previousNode = node;
        }
        linkPaths.push_back(linkPath);
    }
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkLinks = collectNetworkLinks(of);
        int n = networkLinks.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        do {
            std::vector<std::string> failed;
            for (int i = 0; i < n; i++)
                if (mask[i])
                    failed.push_back(networkLinks[i]);
            bool isProtected = false;
            for (int i = 0; i < linkPaths.size(); i++) {
                if (!intersects(linkPaths[i], failed)) {
                    isProtected = true;
                    break;
                }
            }
            if (!isProtected)
                return false;
        } while (std::prev_permutation(mask.begin(), mask.end()));
    }
    return true;
}

void TsnConfigurator::configureStreams()
{
    auto streamRedundancyConfigurator = check_and_cast<StreamRedundancyConfigurator *>(findModuleByPath(par("streamRedundancyConfiguratorModule")));
    cValueArray *streamsParameterValue = new cValueArray();
    for (auto& streamConfiguration : streamConfigurations) {
        cValueMap *streamParameterValue = new cValueMap();
        cValueArray *pathsParameterValue = new cValueArray();
        streamParameterValue->set("name", streamConfiguration.name.c_str());
        streamParameterValue->set("packetFilter", streamConfiguration.packetFilter.c_str());
        streamParameterValue->set("source", streamConfiguration.source.c_str());
        streamParameterValue->set("destination", streamConfiguration.destination.c_str());
        for (auto& path : streamConfiguration.paths) {
            cValueArray *pathParameterValue = new cValueArray();
            for (int i = 0; i < path.size(); i++) {
                // skip source and destination in the alternative paths because they are implied
                if (i != 0 && i != path.size() - 1) {
                    auto name = path[i];
                    pathParameterValue->add(name);
                }
            }
            pathsParameterValue->add(pathParameterValue);
        }
        streamParameterValue->set("paths", pathsParameterValue);
        streamsParameterValue->add(streamParameterValue);
    }
    EV_INFO << "Configuring stream configurator" << EV_FIELD(streamRedundancyConfigurator) << EV_FIELD(streamsParameterValue) << EV_ENDL;
    streamRedundancyConfigurator->par("configuration") = streamsParameterValue;
    auto gateSchedulingConfigurator = findModuleByPath(par("gateSchedulingConfiguratorModule"));
    cValueArray *parameterValue = new cValueArray();
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto source = streamConfiguration->get("source").stringValue();
        auto destination = streamConfiguration->get("destination").stringValue();
        auto pathFragments = streamRedundancyConfigurator->getPathFragments(streamConfiguration->get("name").stringValue());
        cValueMap *streamParameterValue = new cValueMap();
        cValueArray *pathFragmentsParameterValue = new cValueArray();
        for (auto& pathFragment : pathFragments) {
            cValueArray *pathFragmentParameterValue = new cValueArray();
            for (auto nodeName : pathFragment)
                pathFragmentParameterValue->add(nodeName);
            pathFragmentsParameterValue->add(pathFragmentParameterValue);
        }
        streamParameterValue->set("pathFragments", pathFragmentsParameterValue);
        streamParameterValue->set("name", streamConfiguration->get("name").stringValue());
        streamParameterValue->set("application", streamConfiguration->get("application"));
        streamParameterValue->set("source", source);
        streamParameterValue->set("destination", destination);
        streamParameterValue->set("priority", streamConfiguration->get("priority").intValue());
        streamParameterValue->set("packetLength", cValue(streamConfiguration->get("packetLength").doubleValueInUnit("B"), "B"));
        streamParameterValue->set("packetInterval", cValue(streamConfiguration->get("packetInterval").doubleValueInUnit("s"), "s"));
        streamParameterValue->set("maxLatency", cValue(streamConfiguration->get("maxLatency").doubleValueInUnit("s"), "s"));
        parameterValue->add(streamParameterValue);
    }
    gateSchedulingConfigurator->par("configuration") = parameterValue;
}

void TsnConfigurator::collectAllPaths(Node *source, Node *destination, std::vector<std::vector<std::string>>& paths)
{
    std::vector<std::string> current;
    collectAllPaths(source, destination, destination, paths, current);
}

void TsnConfigurator::collectAllPaths(Node *source, Node *destination, Node *node, std::vector<std::vector<std::string>>& paths, std::vector<std::string>& current)
{
    auto networkNodeName = node->module->getFullName();
    current.push_back(networkNodeName);
    if (node == source) {
        paths.push_back(current);
        std::reverse(paths.back().begin(), paths.back().end());
    }
    else {
        for (int i = 0; i < node->getNumPaths(); i++) {
            auto nextNode = (Node *)node->getPath(i)->getRemoteNode();
            std::string nextNetworkNodeName = nextNode->module->getFullName();
            if (!contains(current, nextNetworkNodeName))
                collectAllPaths(source, destination, nextNode, paths, current);
        }
    }
    current.erase(current.end() - 1);
}

std::vector<std::string> TsnConfigurator::collectNetworkNodes(std::string filter)
{
    std::vector<std::string> result;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto name = node->module->getFullName();
        if (matchesFilter(name, filter))
            result.push_back(name);
    }
    return result;
}

std::vector<std::string> TsnConfigurator::collectNetworkLinks(std::string filter)
{
    std::vector<std::string> result;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto localNode = (Node *)topology.getNode(i);
        std::string localName = localNode->module->getFullName();
        for (int j = 0; j < localNode->getNumOutLinks(); j++) {
            auto remoteNode = (Node *)localNode->getLinkOut(j)->getRemoteNode();
            std::string remoteName = remoteNode->module->getFullName();
            std::string linkName = localName + "->" + remoteName;
            if (matchesFilter(linkName.c_str(), filter))
                result.push_back(linkName);
        }
    }
    return result;
}

bool TsnConfigurator::matchesFilter(std::string name, std::string filter)
{
    cMatchExpression matchExpression;
    matchExpression.setPattern(filter.c_str(), false, false, true);
    cMatchableString matchableString(name.c_str());
    return matchExpression.matches(&matchableString);
}

bool TsnConfigurator::intersects(std::vector<std::string> list1, std::vector<std::string> list2)
{
    for (auto element : list1)
        if (contains(list2, element))
            return true;
    return false;
}

Topology::LinkOut *TsnConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return nullptr;
}

TsnConfigurator::InterfaceInfo *TsnConfigurator::findInterfaceInfo(Node *node, NetworkInterface *networkInterface)
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interfaceInfo : node->interfaceInfos)
        if (interfaceInfo->networkInterface == networkInterface)
            return interfaceInfo;

    return nullptr;
}

} // namespace inet

