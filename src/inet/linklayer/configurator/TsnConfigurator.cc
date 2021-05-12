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

#include "inet/common/MatchableObject.h"
#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"

namespace inet {

Define_Module(TsnConfigurator);

void TsnConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureStreams();
    }
}

void TsnConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology));
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
    auto allTrees = collectAllTrees(sourceNode, destinationNodes);
    streamConfiguration.trees = selectBestTreeSubset(configuration, sourceNode, destinationNodes, allTrees);
    EV_INFO << "Found smallest subset of trees with best cost that are sufficient for node and link failure protection" << EV_ENDL;
    for (auto tree : streamConfiguration.trees)
        EV_INFO << "  " << tree << std::endl;
    streamConfigurations.push_back(streamConfiguration);
}

std::vector<TsnConfigurator::Tree> TsnConfigurator::selectBestTreeSubset(cValueMap *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    cValueArray *nodeFailureProtection = configuration->containsKey("nodeFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("nodeFailureProtection").objectValue()) : nullptr;
    cValueArray *linkFailureProtection = configuration->containsKey("linkFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("linkFailureProtection").objectValue()) : nullptr;
    int n = trees.size();
    int maxRedundancy = configuration->containsKey("maxRedundancy") ? configuration->get("maxRedundancy").intValue() : n;
    for (int k = 1; k <= maxRedundancy; k++) {
        EV_INFO << "Trying to find best tree subset for " << k << " trees" << EV_ENDL;
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

double TsnConfigurator::computeTreeCost(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree) const
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
                if (path.nodes[0] == startNode) {
                    for (int i = 0; i < path.nodes.size(); i++) {
                        auto node = path.nodes[i];
                        if (node == destinationNode) {
                            cost += startCost + i;
                            goto nextDestinationNode;
                        }
                        if (node != startNode)
                            todoNodes.push_back({node, startCost + i});
                    }
                }
            }
        }
        nextDestinationNode:;
    }
    return cost;
}

TsnConfigurator::Tree TsnConfigurator::computeCanonicalTree(const Tree& tree) const
{
    Tree canonicalTree({});
    for (auto& path : tree.paths) {
        auto startNode = path.nodes[0];
        auto itStart = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
            return path.nodes.front() == startNode;
        });
        if (itStart != canonicalTree.paths.end())
            // just insert the path into the tree
            canonicalTree.paths.push_back(path);
        else {
            auto itEnd = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
                return path.nodes.back() == startNode;
            });
            if (itEnd != canonicalTree.paths.end()) {
                // append the path to a path that is already in the tree
                auto& canonicalPath = *itEnd;
                canonicalPath.nodes.insert(canonicalPath.nodes.end(), path.nodes.begin() + 1, path.nodes.end());
            }
            else {
                auto itMiddle = std::find_if(canonicalTree.paths.begin(), canonicalTree.paths.end(), [&] (auto path) {
                    return std::find(path.nodes.begin(), path.nodes.end(), startNode) != path.nodes.end();
                });
                if (itMiddle == canonicalTree.paths.end())
                    // just insert the path into the tree
                    canonicalTree.paths.push_back(path);
                else {
                    // split an existing path that is already in the tree and insert the path into the tree
                    auto& canonicalPath = *itMiddle;
                    auto it = std::find(canonicalPath.nodes.begin(), canonicalPath.nodes.end(), startNode);
                    Path firstPathFragment({});
                    Path secondPathFragment({});
                    firstPathFragment.nodes.insert(firstPathFragment.nodes.begin(), canonicalPath.nodes.begin(), it + 1);
                    secondPathFragment.nodes.insert(secondPathFragment.nodes.begin(), it, canonicalPath.nodes.end());
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

bool TsnConfigurator::checkNodeFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    EV_INFO << "Checking trees for node failure protection" << EV_ENDL;
    for (auto& tree : trees)
        EV_INFO << "  " << tree << std::endl;
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkNodes = collectNetworkNodes(of);
        int n = networkNodes.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        EV_INFO << "Checking node failure protection for " << k << " failed nodes out of " << networkNodes.size() << " nodes" << EV_ENDL;
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
        EV_INFO << "Node failure protection succeeded for " << k << " failed nodes out of " << networkNodes.size() << " nodes" << EV_ENDL;
    }
    return true;
}

bool TsnConfigurator::checkLinkFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    EV_INFO << "Checking trees for link failure protection" << EV_ENDL;
    for (auto& tree : trees)
        EV_INFO << "  " << tree << std::endl;
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *protection = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto of = protection->containsKey("of") ? protection->get("of").stringValue() : "*";
        auto networkLinks = collectNetworkLinks(of);
        int n = networkLinks.size();
        int k = protection->get("any").intValue();
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        EV_INFO << "Checking link failure protection for " << k << " failed links out of " << networkLinks.size() << " links" << EV_ENDL;
        do {
            EV_DEBUG << "Assuming failed links: ";
            std::vector<const Link *> failedLinks;
            bool first = true;
            for (int i = 0; i < n; i++) {
                if (mask[i]) {
                    auto link = (Topology::LinkOut *)networkLinks[i];
                    if (!first) { EV_DEBUG << ", "; first = false; }
                    EV_DEBUG << ((Node *)link->getLocalNode())->module->getFullName() << " -> " << ((Node *)link->getRemoteNode())->module->getFullName();
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
        EV_INFO << "Link failure protection succeeded for " << k << " failed links out of " << networkLinks.size() << " links" << EV_ENDL;
    }
    return true;
}

void TsnConfigurator::configureStreams() const
{
    const char *streamRedundancyConfiguratorModulePath = par("streamRedundancyConfiguratorModule");
    if (strlen(streamRedundancyConfiguratorModulePath) != 0) {
        auto streamRedundancyConfigurator = check_and_cast<StreamRedundancyConfigurator *>(getModuleByPath(streamRedundancyConfiguratorModulePath));
        cValueArray *streamsParameterValue = new cValueArray();
        for (auto& streamConfiguration : streamConfigurations) {
            cValueMap *streamParameterValue = new cValueMap();
            cValueArray *treesParameterValue = new cValueArray();
            streamParameterValue->set("name", streamConfiguration.name.c_str());
            streamParameterValue->set("packetFilter", streamConfiguration.packetFilter.c_str());
            streamParameterValue->set("source", streamConfiguration.source.c_str());
            streamParameterValue->set("destination", streamConfiguration.destinations[0].c_str());
            for (auto& tree : streamConfiguration.trees) {
                cValueArray *treeParameterValue = new cValueArray();
                for (auto& path : tree.paths) {
                    cValueArray *pathParameterValue = new cValueArray();
                    for (auto& node : path.nodes)
                        pathParameterValue->add(node->module->getFullName());
                    treeParameterValue->add(pathParameterValue);
                }
                treesParameterValue->add(treeParameterValue);
            }
            streamParameterValue->set("trees", treesParameterValue);
            streamsParameterValue->add(streamParameterValue);
        }
        EV_INFO << "Configuring stream configurator" << EV_FIELD(streamRedundancyConfigurator) << EV_FIELD(streamsParameterValue) << EV_ENDL;
        streamRedundancyConfigurator->par("configuration") = streamsParameterValue;
        const char *gateSchedulingConfiguratorModulePath = par("gateSchedulingConfiguratorModule");
        if (strlen(gateSchedulingConfiguratorModulePath) != 0) {
            auto gateSchedulingConfigurator = getModuleByPath(gateSchedulingConfiguratorModulePath);
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
    }
}

std::vector<TsnConfigurator::Tree> TsnConfigurator::collectAllTrees(Node *sourceNode, const std::vector<const Node *>& destinationNodes) const
{
    std::vector<Tree> allTrees;
    topology->calculateUnweightedSingleShortestPathsTo(sourceNode);
    std::vector<Path> currentTree;
    std::vector<const Node *> stopNodes;
    stopNodes.push_back(sourceNode);
    EV_INFO << "Collecting all possible trees" << EV_ENDL;
    collectAllTrees(stopNodes, destinationNodes, 0, currentTree, allTrees);
    for (auto tree : allTrees)
        EV_INFO << "  " << tree << std::endl;
    return allTrees;
}

void TsnConfigurator::collectAllTrees(const std::vector<const Node *>& stopNodes, const std::vector<const Node *>& destinationNodes, int destinationNodeIndex, std::vector<Path>& currentTree, std::vector<Tree>& allTrees) const
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
                destinationStopNodes.insert(destinationStopNodes.end(), path.nodes.begin(), path.nodes.end());
                currentTree.push_back(path);
                collectAllTrees(destinationStopNodes, destinationNodes, destinationNodeIndex + 1, currentTree, allTrees);
                currentTree.erase(currentTree.end() - 1);
            }
        }
    }
}

std::vector<TsnConfigurator::Path> TsnConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *destinationNode) const
{
    std::vector<Path> allPaths;
    std::vector<const Node *> currentPath;
    collectAllPaths(stopNodes, destinationNode, currentPath, allPaths);
    return allPaths;
}

void TsnConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *currentNode, std::vector<const Node *>& currentPath, std::vector<Path>& allPaths) const
{
    currentPath.push_back(currentNode);
    if (std::find(stopNodes.begin(), stopNodes.end(), currentNode) != stopNodes.end()) {
        allPaths.push_back(Path(currentPath));
        std::reverse(allPaths.back().nodes.begin(), allPaths.back().nodes.end());
    }
    else {
        for (int i = 0; i < currentNode->getNumPaths(); i++) {
            auto nextNode = (Node *)currentNode->getPath(i)->getRemoteNode();
            if (std::find(currentPath.begin(), currentPath.end(), nextNode) == currentPath.end())
                collectAllPaths(stopNodes, nextNode, currentPath, allPaths);
        }
    }
    currentPath.erase(currentPath.end() - 1);
}

std::vector<const TsnConfigurator::Node *> TsnConfigurator::collectNetworkNodes(const std::string& filter) const
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

std::vector<const TsnConfigurator::Link *> TsnConfigurator::collectNetworkLinks(const std::string& filter) const
{
    std::vector<const Link *> result;
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto localNode = (Node *)topology->getNode(i);
        std::string localName = localNode->module->getFullName();
        for (int j = 0; j < localNode->getNumOutLinks(); j++) {
            auto link = localNode->getLinkOut(j);
            auto remoteNode = (Node *)link->getRemoteNode();
            std::string remoteName = remoteNode->module->getFullName();
            std::string linkName = localName + "->" + remoteName;
            if (matchesFilter(linkName.c_str(), filter))
                result.push_back((Link *)link);
        }
    }
    return result;
}

void TsnConfigurator::collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Node *>& failedNodes, std::vector<bool>& reachedDestinationNodes) const
{
    std::deque<const Node *> todoNodes;
    todoNodes.push_back(sourceNode);
    while (!todoNodes.empty()) {
        auto startNode = todoNodes.front();
        todoNodes.pop_front();
        for (auto path : tree.paths) {
            if (path.nodes[0] == startNode) {
                for (auto node : path.nodes) {
                    if (std::find(failedNodes.begin(), failedNodes.end(), node) != failedNodes.end())
                        break;
                    auto it = std::find(destinationNodes.begin(), destinationNodes.end(), node);
                    if (it != destinationNodes.end())
                        reachedDestinationNodes[it - destinationNodes.begin()] = true;
                    if (node != startNode)
                        todoNodes.push_back(node);
                }
            }
        }
    }
}

void TsnConfigurator::collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Link *>& failedLinks, std::vector<bool>& reachedDestinationNodes) const
{
    std::deque<const Node *> todoNodes;
    todoNodes.push_back(sourceNode);
    while (!todoNodes.empty()) {
        auto startNode = todoNodes.front();
        todoNodes.pop_front();
        for (auto path : tree.paths) {
            if (path.nodes[0] == startNode) {
                const Node *previousNode = nullptr;
                for (auto node : path.nodes) {
                    if (previousNode != nullptr) {
                        Link *pathLink = findLinkOut(previousNode, node);
                        if (std::find(failedLinks.begin(), failedLinks.end(), pathLink) != failedLinks.end())
                            break;
                    }
                    auto it = std::find(destinationNodes.begin(), destinationNodes.end(), node);
                    if (it != destinationNodes.end())
                        reachedDestinationNodes[it - destinationNodes.begin()] = true;
                    if (node != startNode)
                        todoNodes.push_back(node);
                    previousNode = node;
                }
            }
        }
    }
}

bool TsnConfigurator::matchesFilter(const std::string& name, const std::string& filter) const
{
    cMatchExpression matchExpression;
    matchExpression.setPattern(filter.c_str(), false, false, true);
    cMatchableString matchableString(name.c_str());
    return matchExpression.matches(&matchableString);
}

} // namespace inet

