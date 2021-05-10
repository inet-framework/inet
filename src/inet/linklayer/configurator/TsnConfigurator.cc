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
    Node *source = static_cast<Node *>(topology->getNodeFor(getParentModule()->getSubmodule(sourceNetworkNodeName)));
    std::vector<Node *> destinations;
    cMatchExpression destinationFilter;
    destinationFilter.setPattern(configuration->get("destination").stringValue(), false, false, true);
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, node->module);
        if (destinationFilter.matches(&matchableObject)) {
            destinations.push_back(node);
            streamConfiguration.destinations.push_back(node->module->getFullName());
        }
    }
    auto allTrees = collectAllTrees(source, destinations);
    streamConfiguration.trees = selectBestTreeSubset(configuration, allTrees);
    streamConfigurations.push_back(streamConfiguration);
}

std::vector<TsnConfigurator::Tree> TsnConfigurator::selectBestTreeSubset(cValueMap *configuration, const std::vector<Tree>& trees)
{
    cValueArray *nodeFailureProtection = configuration->containsKey("nodeFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("nodeFailureProtection").objectValue()) : nullptr;
    cValueArray *linkFailureProtection = configuration->containsKey("linkFailureProtection") ? check_and_cast<cValueArray *>(configuration->get("linkFailureProtection").objectValue()) : nullptr;
    int n = trees.size();
    for (int k = 1; k <= trees.size(); k++) {
        std::vector<bool> mask(k, true); // k leading 1's
        mask.resize(n, false); // n-k trailing 0's
        std::vector<bool> bestMask;
        double bestCost = DBL_MAX;
        do {
            double cost = 0;
            for (int i = 0; i < n; i++)
                if (mask[i])
                    cost += trees[i].paths[0].nodes.size();
            cost /= k;
            if (cost < bestCost) {
                std::vector<Tree> candidate;
                for (int i = 0; i < n; i++)
                    if (mask[i])
                        candidate.push_back(trees[i]);
                if ((nodeFailureProtection == nullptr || checkNodeFailureProtection(nodeFailureProtection, candidate)) &&
                    (linkFailureProtection == nullptr || checkLinkFailureProtection(linkFailureProtection, candidate)))
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

bool TsnConfigurator::checkNodeFailureProtection(cValueArray *configuration, const std::vector<Tree>& trees)
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
            for (int i = 0; i < trees.size(); i++) {
                if (!intersects(trees[i].paths[0].nodes, failed)) {
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

bool TsnConfigurator::checkLinkFailureProtection(cValueArray *configuration, const std::vector<Tree>& trees)
{
    std::vector<Tree> linkTrees;
    for (auto tree : trees) {
        std::vector<std::string> linkPath;
        std::string previousNode;
        for (int i = 0; i < tree.paths[0].nodes.size(); i++) {
            auto node = tree.paths[0].nodes[i];
            if (!previousNode.empty()) {
                auto link = previousNode + "->" + node;
                linkPath.push_back(link);
            }
            previousNode = node;
        }
        linkTrees.push_back(Tree({Path(linkPath)}));
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
            for (int i = 0; i < linkTrees.size(); i++) {
                if (!intersects(linkTrees[i].paths[0].nodes, failed)) {
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
                for (int i = 0; i < tree.paths[0].nodes.size(); i++) {
                    // skip source and destination in the alternative paths because they are implied
                    if (i != 0 && i != tree.paths[0].nodes.size() - 1) {
                        auto name = tree.paths[0].nodes[i];
                        treeParameterValue->add(name);
                    }
                }
                treesParameterValue->add(treeParameterValue);
            }
            streamParameterValue->set("paths", treesParameterValue);
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

std::vector<TsnConfigurator::Tree> TsnConfigurator::collectAllTrees(Node *source, std::vector<Node *> destinations)
{
    std::vector<Tree> trees;
    std::vector<std::string> current;
    topology->calculateUnweightedSingleShortestPathsTo(source);
    collectAllTrees(source, destinations, destinations[0], trees, current);
    return trees;
}

void TsnConfigurator::collectAllTrees(Node *source, std::vector<Node *> destinations, Node *node, std::vector<Tree>& trees, std::vector<std::string>& current)
{
    auto networkNodeName = node->module->getFullName();
    current.push_back(networkNodeName);
    if (node == source) {
        trees.push_back(Tree({Path(current)}));
        std::reverse(trees.back().paths[0].nodes.begin(), trees.back().paths[0].nodes.end());
    }
    else {
        for (int i = 0; i < node->getNumPaths(); i++) {
            auto nextNode = (Node *)node->getPath(i)->getRemoteNode();
            auto nextNetworkNodeName = nextNode->module->getFullName();
            if (std::find(current.begin(), current.end(), nextNetworkNodeName) == current.end())
                collectAllTrees(source, destinations, nextNode, trees, current);
        }
    }
    current.erase(current.end() - 1);
}

std::vector<std::string> TsnConfigurator::collectNetworkNodes(std::string filter)
{
    std::vector<std::string> result;
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto name = node->module->getFullName();
        if (matchesFilter(name, filter))
            result.push_back(name);
    }
    return result;
}

std::vector<std::string> TsnConfigurator::collectNetworkLinks(std::string filter)
{
    std::vector<std::string> result;
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto localNode = (Node *)topology->getNode(i);
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

} // namespace inet

