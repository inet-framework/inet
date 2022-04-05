//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/FailureProtectionConfigurator.h"

#include "inet/common/MatchableObject.h"
#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"

namespace inet {

Define_Module(FailureProtectionConfigurator);

void FailureProtectionConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureStreams();
    }
}

void FailureProtectionConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology));
    TIME(computeStreams());
    printElapsedTime("initialize", initializeStartTime);
}

void FailureProtectionConfigurator::computeStreams()
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        computeStream(streamConfiguration);
    }
}

void FailureProtectionConfigurator::computeStream(cValueMap *configuration)
{
    auto streamName = configuration->get("name").stringValue();
    StreamConfiguration streamConfiguration;
    streamConfiguration.name = streamName;
    if (configuration->containsKey("pcp"))
        streamConfiguration.pcp = configuration->get("pcp").intValue();
    if (configuration->containsKey("gateIndex"))
        streamConfiguration.gateIndex = configuration->get("gateIndex").intValue();
    streamConfiguration.packetFilter = configuration->get("packetFilter");
    auto sourceNetworkNodeName = configuration->get("source").stringValue();
    streamConfiguration.source = sourceNetworkNodeName;
    Node *sourceNode = static_cast<Node *>(topology->getNodeFor(getParentModule()->getSubmodule(sourceNetworkNodeName)));
    std::vector<const Node *> destinationNodes;
    cMatchExpression destinationFilter;
    destinationFilter.setPattern(configuration->get("destination").stringValue(), false, false, true);
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, node->module);
        if (destinationFilter.matches(&matchableObject)) {
            destinationNodes.push_back(node);
            streamConfiguration.destinations.push_back(node->module->getFullName());
        }
    }
    auto sourceNodeName = sourceNode->module->getFullName();
    std::stringstream destinationNodeNames;
    destinationNodeNames << "[";
    for (int i = 0; i < destinationNodes.size(); i++) {
        if (i != 0)
            destinationNodeNames << ", ";
        destinationNodeNames << destinationNodes[i]->module->getFullName();
    }
    destinationNodeNames << "]";
    EV_INFO << "Computing stream configuration" << EV_FIELD(streamName) << EV_FIELD(sourceNodeName) << EV_FIELD(destinationNodeNames) << EV_ENDL;
    streamConfiguration.destinationAddress = configuration->containsKey("destinationAddress") ? configuration->get("destinationAddress").stringValue() : streamConfiguration.destinations[0];
    EV_INFO << "Collecting all possible trees" << EV_FIELD(streamName) << EV_ENDL;
    auto allTrees = collectAllTrees(sourceNode, destinationNodes);
    for (auto tree : allTrees)
        EV_INFO << "  Found tree" << EV_FIELD(streamName) << EV_FIELD(tree) << std::endl;
    EV_INFO << "Selecting best tree subset" << EV_FIELD(streamName) << EV_ENDL;
    streamConfiguration.trees = selectBestTreeSubset(configuration, sourceNode, destinationNodes, allTrees);
    EV_INFO << "Smallest tree subset having the best cost that still provide failure protection" << EV_FIELD(streamName) << EV_FIELD(sourceNodeName) << EV_FIELD(destinationNodeNames) << EV_ENDL;
    for (auto tree : streamConfiguration.trees)
        EV_INFO << "  Selected tree" << EV_FIELD(streamName) << EV_FIELD(tree) << std::endl;
    streamConfigurations.push_back(streamConfiguration);
}

std::vector<FailureProtectionConfigurator::Tree> FailureProtectionConfigurator::selectBestTreeSubset(cValueMap *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    cValueArray *nodeFailureProtection = configuration->containsKey("nodeFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("nodeFailureProtection").objectValue()) : nullptr;
    cValueArray *linkFailureProtection = configuration->containsKey("linkFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("linkFailureProtection").objectValue()) : nullptr;
    int n = trees.size();
    int maxRedundancy = configuration->containsKey("maxRedundancy") ? configuration->get("maxRedundancy").intValue() : n;
    for (int k = 1; k <= maxRedundancy; k++) {
        EV_DETAIL << "Trying to find best tree subset for " << k << " trees" << EV_ENDL;
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        std::vector<bool> bestMask;
        double bestCost = DBL_MAX;
        do {
            double cost = 0;
            for (int i = 0; i < n; i++)
                if (mask[i])
                    cost += computeTreeCost(sourceNode, destinationNodes, trees[i]);
            cost /= k;
            if (cost < bestCost) {
                std::vector<Tree> candidate;
                for (int i = 0; i < n; i++)
                    if (mask[i])
                        candidate.push_back(trees[i]);
                if ((nodeFailureProtection == nullptr || checkNodeFailureProtection(nodeFailureProtection, sourceNode, destinationNodes, candidate)) &&
                    (linkFailureProtection == nullptr || checkLinkFailureProtection(linkFailureProtection, sourceNode, destinationNodes, candidate)))
                {
                    bestCost = cost;
                    bestMask = mask;
                }
            }
        } while (std::prev_permutation(mask.begin(), mask.end()));
        if (bestCost != DBL_MAX) {
            std::vector<Tree> result;
            for (int i = 0; i < n; i++)
                if (bestMask[i])
                    result.push_back(trees[i]);
            return result;
        }
    }
    throw cRuntimeError("Cannot find tree combination that protects against all configured node and link failures");
}

double FailureProtectionConfigurator::computeTreeCost(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree) const
{
    double cost = 0;
    // sum up the total number of links in the shortest paths to all destinations
    for (auto destinationNode : destinationNodes) {
        std::deque<std::pair<const Node *, int>> todoNodes;
        todoNodes.push_back({sourceNode, 0});
        while (!todoNodes.empty()) {
            auto it = todoNodes.front();
            todoNodes.pop_front();
            auto startNode = it.first;
            auto startCost = it.second;
            for (auto path : tree.paths) {
                if (path.interfaces[0]->node == startNode) {
                    for (int i = 0; i < path.interfaces.size(); i++) {
                        auto interface = path.interfaces[i];
                        if (interface->node == destinationNode) {
                            cost += startCost + i;
                            goto nextDestinationNode;
                        }
                        if (interface->node != startNode)
                            todoNodes.push_back({interface->node, startCost + i});
                    }
                }
            }
        }
        nextDestinationNode:;
    }
    return cost;
}

FailureProtectionConfigurator::Tree FailureProtectionConfigurator::computeCanonicalTree(const Tree& tree) const
{
    Tree canonicalTree({});
    for (auto& path : tree.paths) {
        auto startInterface = path.interfaces[0];
        auto itStart = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
            return path.interfaces.front()->node == startInterface->node;
        });
        if (itStart != canonicalTree.paths.end())
            // just insert the path into the tree
            canonicalTree.paths.push_back(path);
        else {
            auto itEnd = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
                return path.interfaces.back()->node == startInterface->node;
            });
            if (itEnd != canonicalTree.paths.end()) {
                // append the path to a path that is already in the tree
                auto& canonicalPath = *itEnd;
                canonicalPath.interfaces.insert(canonicalPath.interfaces.end(), path.interfaces.begin() + 1, path.interfaces.end());
            }
            else {
                auto itMiddle = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
                    auto jt = std::find_if(path.interfaces.begin(), path.interfaces.end(), [&] (auto interface) {
                        return interface->node == startInterface->node;
                    });
                    return jt != path.interfaces.end();
                });
                if (itMiddle == canonicalTree.paths.end())
                    // just insert the path into the tree
                    canonicalTree.paths.push_back(path);
                else {
                    // split an existing path that is already in the tree and insert the path into the tree
                    auto& canonicalPath = *itMiddle;
                    auto it = std::find_if(canonicalPath.interfaces.begin(), canonicalPath.interfaces.end(), [&] (auto interface) {
                        return interface->node == startInterface->node;
                    });
                    Path firstPathFragment({});
                    Path secondPathFragment({});
                    firstPathFragment.interfaces.insert(firstPathFragment.interfaces.begin(), canonicalPath.interfaces.begin(), it + 1);
                    secondPathFragment.interfaces.insert(secondPathFragment.interfaces.begin(), it, canonicalPath.interfaces.end());
                    canonicalTree.paths.erase(itMiddle);
                    canonicalTree.paths.push_back(firstPathFragment);
                    canonicalTree.paths.push_back(secondPathFragment);
                    canonicalTree.paths.push_back(path);
                }
            }
        }
    }
    return canonicalTree;
}

bool FailureProtectionConfigurator::checkNodeFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    EV_DETAIL << "Checking trees for node failure protection" << EV_ENDL;
    for (auto& tree : trees)
        EV_DETAIL << "  " << tree << std::endl;
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkNodes = collectNetworkNodes(of);
        int n = networkNodes.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        EV_DETAIL << "Checking node failure protection for " << k << " failed nodes out of " << networkNodes.size() << " nodes" << EV_ENDL;
        do {
            EV_DEBUG << "Assuming failed nodes: ";
            std::vector<const Node *> failedNodes;
            bool first = true;
            for (int i = 0; i < n; i++) {
                if (mask[i]) {
                    auto node = networkNodes[i];
                    if (!first) { EV_DEBUG << ", "; first = false; }
                    EV_DEBUG << node->module->getFullName();
                    failedNodes.push_back(node);
                }
            }
            EV_DEBUG << std::endl;
            std::vector<bool> reachedDestinationNodes;
            reachedDestinationNodes.resize(destinationNodes.size(), false);
            for (int i = 0; i < trees.size(); i++)
                collectReachedNodes(sourceNode, destinationNodes, trees[i], failedNodes, reachedDestinationNodes);
            bool isProtected = std::all_of(reachedDestinationNodes.begin(), reachedDestinationNodes.end(), [] (bool v) { return v; });
            EV_DEBUG << "Node failure protection " << (isProtected ? "succeeded" : "failed") << EV_ENDL;
            if (!isProtected)
                return false;
        } while (std::prev_permutation(mask.begin(), mask.end()));
        EV_DETAIL << "Node failure protection succeeded for " << k << " failed nodes out of " << networkNodes.size() << " nodes" << EV_ENDL;
    }
    return true;
}

bool FailureProtectionConfigurator::checkLinkFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    EV_DETAIL << "Checking trees for link failure protection" << EV_ENDL;
    for (auto& tree : trees)
        EV_DETAIL << "  " << tree << std::endl;
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkLinks = collectNetworkLinks(of);
        int n = networkLinks.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        EV_DETAIL << "Checking link failure protection for " << k << " failed links out of " << networkLinks.size() << " links" << EV_ENDL;
        do {
            EV_DEBUG << "Assuming failed links: ";
            std::vector<const Link *> failedLinks;
            bool first = true;
            for (int i = 0; i < n; i++) {
                if (mask[i]) {
                    auto link = (Link *)networkLinks[i];
                    if (!first) { EV_DEBUG << ", "; first = false; }
                    EV_DEBUG << link->sourceInterface->node->module->getFullName() << "." << link->sourceInterface->networkInterface->getInterfaceName() << " -> " << link->destinationInterface->node->module->getFullName() << "." << link->destinationInterface->networkInterface->getInterfaceName();
                    failedLinks.push_back(networkLinks[i]);
                }
            }
            EV_DEBUG << std::endl;
            std::vector<bool> reachedDestinationNodes;
            reachedDestinationNodes.resize(destinationNodes.size(), false);
            for (int i = 0; i < trees.size(); i++)
                collectReachedNodes(sourceNode, destinationNodes, trees[i], failedLinks, reachedDestinationNodes);
            bool isProtected = std::all_of(reachedDestinationNodes.begin(), reachedDestinationNodes.end(), [] (bool v) { return v; });
            EV_DEBUG << "Link failure protection " << (isProtected ? "succeeded" : "failed") << EV_ENDL;
            if (!isProtected)
                return false;
        } while (std::prev_permutation(mask.begin(), mask.end()));
        EV_DETAIL << "Link failure protection succeeded for " << k << " failed links out of " << networkLinks.size() << " links" << EV_ENDL;
    }
    return true;
}

void FailureProtectionConfigurator::configureStreams() const
{
    const char *streamRedundancyConfiguratorModulePath = par("streamRedundancyConfiguratorModule");
    if (strlen(streamRedundancyConfiguratorModulePath) != 0) {
        auto streamRedundancyConfigurator = check_and_cast<StreamRedundancyConfigurator *>(getModuleByPath(streamRedundancyConfiguratorModulePath));
        cValueArray *streamsParameterValue = new cValueArray();
        for (auto& streamConfiguration : streamConfigurations) {
            cValueMap *streamParameterValue = new cValueMap();
            cValueArray *treesParameterValue = new cValueArray();
            streamParameterValue->set("name", streamConfiguration.name.c_str());
            streamParameterValue->set("pcp", streamConfiguration.pcp);
            streamParameterValue->set("packetFilter", streamConfiguration.packetFilter);
            streamParameterValue->set("source", streamConfiguration.source.c_str());
            // TODO KLUDGE
            streamParameterValue->set("destination", streamConfiguration.destinations[0].c_str());
            streamParameterValue->set("destinationAddress", streamConfiguration.destinationAddress.c_str());
            for (auto& tree : streamConfiguration.trees) {
                cValueArray *treeParameterValue = new cValueArray();
                for (auto& path : tree.paths) {
                    cValueArray *pathParameterValue = new cValueArray();
                    for (auto& interface : path.interfaces) {
                        std::string name;
                        name = interface->node->module->getFullName();
                        if (FailureProtectionConfigurator::countParalellLinks(interface) > 1)
                            name = name + "." + interface->networkInterface->getInterfaceName();
                        pathParameterValue->add(name.c_str());
                    }
                    treeParameterValue->add(pathParameterValue);
                }
                treesParameterValue->add(treeParameterValue);
            }
            streamParameterValue->set("trees", treesParameterValue);
            streamsParameterValue->add(streamParameterValue);
        }
        EV_INFO << "Configuring stream configurator" << EV_FIELD(streamRedundancyConfigurator) << EV_FIELD(streamsParameterValue) << EV_ENDL;
        streamRedundancyConfigurator->par("configuration") = streamsParameterValue;
        const char *gateScheduleConfiguratorModulePath = par("gateScheduleConfiguratorModule");
        if (strlen(gateScheduleConfiguratorModulePath) != 0) {
            auto gateScheduleConfigurator = getModuleByPath(gateScheduleConfiguratorModulePath);
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
                if (streamConfiguration->containsKey("pcp"))
                    streamParameterValue->set("pcp", streamConfiguration->get("pcp").intValue());
                if (streamConfiguration->containsKey("gateIndex"))
                    streamParameterValue->set("gateIndex", streamConfiguration->get("gateIndex").intValue());
                streamParameterValue->set("packetLength", cValue(streamConfiguration->get("packetLength").doubleValueInUnit("B"), "B"));
                streamParameterValue->set("packetInterval", cValue(streamConfiguration->get("packetInterval").doubleValueInUnit("s"), "s"));
                if (streamConfiguration->containsKey("maxLatency"))
                    streamParameterValue->set("maxLatency", cValue(streamConfiguration->get("maxLatency").doubleValueInUnit("s"), "s"));
                parameterValue->add(streamParameterValue);
            }
            gateScheduleConfigurator->par("configuration") = parameterValue;
        }
    }
}

std::vector<FailureProtectionConfigurator::Tree> FailureProtectionConfigurator::collectAllTrees(Node *sourceNode, const std::vector<const Node *>& destinationNodes) const
{
    std::vector<Tree> allTrees;
    topology->calculateUnweightedSingleShortestPathsTo(sourceNode);
    std::vector<Path> currentTree;
    std::vector<const Node *> stopNodes;
    stopNodes.push_back(sourceNode);
    collectAllTrees(stopNodes, destinationNodes, 0, currentTree, allTrees);
    return allTrees;
}

void FailureProtectionConfigurator::collectAllTrees(const std::vector<const Node *>& stopNodes, const std::vector<const Node *>& destinationNodes, int destinationNodeIndex, std::vector<Path>& currentTree, std::vector<Tree>& allTrees) const
{
    if (destinationNodes.size() == destinationNodeIndex)
        allTrees.push_back(computeCanonicalTree(currentTree));
    else {
        auto destinationNode = destinationNodes[destinationNodeIndex];
        if (std::find(stopNodes.begin(), stopNodes.end(), destinationNode) != stopNodes.end())
            collectAllTrees(stopNodes, destinationNodes, destinationNodeIndex + 1, currentTree, allTrees);
        else {
            auto allPaths = collectAllPaths(stopNodes, destinationNode);
            for (auto& path : allPaths) {
                auto destinationStopNodes = stopNodes;
                for (auto interface : path.interfaces)
                    if (std::find(destinationStopNodes.begin(), destinationStopNodes.end(), interface->node) == destinationStopNodes.end())
                        destinationStopNodes.push_back(interface->node);
                currentTree.push_back(path);
                collectAllTrees(destinationStopNodes, destinationNodes, destinationNodeIndex + 1, currentTree, allTrees);
                currentTree.erase(currentTree.end() - 1);
            }
        }
    }
}

std::vector<FailureProtectionConfigurator::Path> FailureProtectionConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *destinationNode) const
{
    std::vector<Path> allPaths;
    std::vector<const Interface *> currentPath;
    collectAllPaths(stopNodes, destinationNode, currentPath, allPaths);
    return allPaths;
}

void FailureProtectionConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *currentNode, std::vector<const Interface *>& currentPath, std::vector<Path>& allPaths) const
{
    if (std::find(stopNodes.begin(), stopNodes.end(), currentNode) != stopNodes.end()) {
        allPaths.push_back(Path(currentPath));
        std::reverse(allPaths.back().interfaces.begin(), allPaths.back().interfaces.end());
    }
    else {
        for (int i = 0; i < currentNode->getNumPaths(); i++) {
            auto link = (Link *)currentNode->getPath(i);
            auto nextNode = (Node *)currentNode->getPath(i)->getLinkOutRemoteNode();
            if (std::find_if(currentPath.begin(), currentPath.end(), [&] (const Interface *interface) { return interface->node == nextNode; }) == currentPath.end()) {
                bool first = currentPath.empty();
                if (first)
                    currentPath.push_back(link->sourceInterface);
                currentPath.push_back(link->destinationInterface);
                collectAllPaths(stopNodes, nextNode, currentPath, allPaths);
                currentPath.erase(currentPath.end() - (first ? 2 : 1), currentPath.end());
            }
        }
    }
}

std::vector<const FailureProtectionConfigurator::Node *> FailureProtectionConfigurator::collectNetworkNodes(const std::string& filter) const
{
    std::vector<const Node *> result;
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto name = node->module->getFullName();
        if (matchesFilter(name, filter))
            result.push_back(node);
    }
    return result;
}

std::vector<const FailureProtectionConfigurator::Link *> FailureProtectionConfigurator::collectNetworkLinks(const std::string& filter) const
{
    std::vector<const Link *> result;
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto localNode = (Node *)topology->getNode(i);
        std::string localName = localNode->module->getFullName();
        for (int j = 0; j < localNode->getNumOutLinks(); j++) {
            auto link = localNode->getLinkOut(j);
            auto remoteNode = (Node *)link->getLinkOutRemoteNode();
            std::string remoteName = remoteNode->module->getFullName();
            std::string linkName = localName + "->" + remoteName;
            if (matchesFilter(linkName.c_str(), filter))
                result.push_back((Link *)link);
        }
    }
    return result;
}

void FailureProtectionConfigurator::collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Node *>& failedNodes, std::vector<bool>& reachedDestinationNodes) const
{
    std::deque<const Node *> todoNodes;
    todoNodes.push_back(sourceNode);
    while (!todoNodes.empty()) {
        auto startNode = todoNodes.front();
        todoNodes.pop_front();
        for (auto path : tree.paths) {
            if (path.interfaces[0]->node == startNode) {
                for (auto interface : path.interfaces) {
                    if (std::find(failedNodes.begin(), failedNodes.end(), interface->node) != failedNodes.end())
                        break;
                    auto it = std::find_if(destinationNodes.begin(), destinationNodes.end(), [&] (const Node *node) { return interface->node == node; });
                    if (it != destinationNodes.end())
                        reachedDestinationNodes[it - destinationNodes.begin()] = true;
                    if (interface->node != startNode)
                        todoNodes.push_back(interface->node);
                }
            }
        }
    }
}

void FailureProtectionConfigurator::collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Link *>& failedLinks, std::vector<bool>& reachedDestinationNodes) const
{
    std::deque<const Node *> todoNodes;
    todoNodes.push_back(sourceNode);
    while (!todoNodes.empty()) {
        auto startNode = todoNodes.front();
        todoNodes.pop_front();
        for (auto path : tree.paths) {
            if (path.interfaces[0]->node == startNode) {
                const Interface *previousInterface = nullptr;
                for (auto interface : path.interfaces) {
                    if (previousInterface != nullptr) {
                        Link *pathLink = findLinkOut(previousInterface);
                        if (pathLink->destinationInterface->node != interface->node)
                            pathLink = findLinkOut(previousInterface->node, interface->node);
                        if (std::find(failedLinks.begin(), failedLinks.end(), pathLink) != failedLinks.end())
                            break;
                    }
                    auto it = std::find_if(destinationNodes.begin(), destinationNodes.end(), [&] (const Node *node) { return interface->node == node; });
                    if (it != destinationNodes.end())
                        reachedDestinationNodes[it - destinationNodes.begin()] = true;
                    if (interface->node != startNode)
                        todoNodes.push_back(interface->node);
                    previousInterface = interface;
                }
            }
        }
    }
}

bool FailureProtectionConfigurator::matchesFilter(const std::string& name, const std::string& filter) const
{
    cMatchExpression matchExpression;
    matchExpression.setPattern(filter.c_str(), false, false, true);
    cMatchableString matchableString(name.c_str());
    return matchExpression.matches(&matchableString);
}

} // namespace inet

