//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/MacForwardingTableConfigurator.h"

#include "inet/common/MatchableObject.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/configurator/MacForwardingTableConfigurator.h"
#include "inet/networklayer/arp/ipv4/GlobalArp.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(MacForwardingTableConfigurator);

MacForwardingTableConfigurator::~MacForwardingTableConfigurator()
{
    for (auto it : macForwardingTables)
        delete it.second;
}

void MacForwardingTableConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureMacForwardingTables();
        getParentModule()->subscribe(ipv4MulticastChangeSignal, this);
    }
}

void MacForwardingTableConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology))
    if (par("addUnicastRules"))
        TIME(computeUnicastRules());
    TIME(computeStreamRules());
    printElapsedTime("initialize", initializeStartTime);
}

void MacForwardingTableConfigurator::computeUnicastRules()
{
    // for each network node as destination
    //   for each network interface of destination
    //   find shortest path from all sources to destination passing through the network interface
    //     for each switch as source
    //       store first outgoing interface of the switch in the MAC forwarding table of the switch
    //       towards the MAC address of the network interface of the destination
    for (int i = 0; i < topology->getNumNodes(); i++) {
        Node *destinationNode = (Node *)topology->getNode(i);
        if (!isBridgeNode(destinationNode)) {
            for (auto destinationInterface : destinationNode->interfaces) {
                if (!destinationInterface->networkInterface->isLoopback() &&
                    !destinationInterface->networkInterface->getMacAddress().isUnspecified())
                {
                    extendConfiguration(destinationNode, destinationInterface, destinationInterface->networkInterface->getMacAddress());
                }
            }
        }
    }
}

void MacForwardingTableConfigurator::extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress)
{
    for (int j = 0; j < destinationNode->getNumInLinks(); j++) {
        auto link = (Link *)destinationNode->getLinkIn(j);
        link->setWeight(link->destinationInterface != destinationInterface ? std::numeric_limits<double>::infinity() : 1);
    }
    topology->calculateWeightedSingleShortestPathsTo(destinationNode);
    for (int j = 0; j < topology->getNumNodes(); j++) {
        Node *sourceNode = (Node *)topology->getNode(j);
        if (sourceNode != destinationNode && isBridgeNode(sourceNode) && sourceNode->getNumPaths() != 0) {
            auto firstLink = (Link *)sourceNode->getPath(0);
            auto firstInterface = static_cast<Interface *>(firstLink->sourceInterface);
            auto interfaceName = firstInterface->networkInterface->getInterfaceName();
            auto moduleId = sourceNode->module->getSubmodule("macTable")->getId();
            auto it = macForwardingTables.find(moduleId);
            if (it == macForwardingTables.end())
                macForwardingTables[moduleId] = new cValueArray();
            else if (findForwardingRule(it->second, macAddress, interfaceName) != nullptr)
                continue;
            else {
                auto rule = new cValueMap();
                rule->set("address", macAddress.str());
                rule->set("interface", interfaceName);
                it->second->add(rule);
            }
        }
    }
}

cValueMap *MacForwardingTableConfigurator::findForwardingRule(cValueArray *configuration, MacAddress macAddress, std::string interfaceName)
{
    for (int i = 0; i < configuration->size(); i++) {
        auto rule = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        if (rule->get("address").stdstringValue() == macAddress.str() &&
            rule->get("interface").stdstringValue() == interfaceName)
        {
            return rule;
        }
    }
    return nullptr;
}

void MacForwardingTableConfigurator::configureMacForwardingTables() const
{
    for (auto it : macForwardingTables) {
        auto macForwardingTable = getSimulation()->getModule(it.first);
        macForwardingTable->par("forwardingTable") = it.second->dup();
    }
}

void MacForwardingTableConfigurator::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == ipv4MulticastChangeSignal) {
        auto sourceInfo = check_and_cast<Ipv4MulticastGroupSourceInfo *>(object);
        auto node = static_cast<Node *>(topology->getNodeFor(getContainingNode(sourceInfo->ie)));
        auto interface = findInterface(node, sourceInfo->ie);
        extendConfiguration(node, interface, sourceInfo->groupAddress.mapToMulticastMacAddress());
        configureMacForwardingTables();
    }
}

void MacForwardingTableConfigurator::computeStreamRules()
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        auto streamTreeConfiguration = computeStreamTree(streamConfiguration);
        computeStreamSendersAndReceivers(streamTreeConfiguration);
        computeStreamRules(streamTreeConfiguration);
    }
}

void MacForwardingTableConfigurator::computeStreamSendersAndReceivers(cValueMap *streamConfiguration)
{
    std::string streamName = streamConfiguration->get("name").stringValue();
    auto& stream = streams[streamName];
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        cValueArray *trees = check_and_cast<cValueArray *>(streamConfiguration->get("trees").objectValue());
        stream.streamNodes[networkNodeName].senders.resize(trees->size());
        stream.streamNodes[networkNodeName].receivers.resize(trees->size());
        stream.streamNodes[networkNodeName].interfaces.resize(trees->size());
        std::vector<std::string> senderNetworkNodeNames;
        for (int j = 0; j < trees->size(); j++) {
            std::vector<std::string> receiverNetworkNodeNames;
            std::vector<NetworkInterface *> interfaces;
            cValueArray *tree = check_and_cast<cValueArray*>(trees->get(j).objectValue());
            for (int k = 0; k < tree->size(); k++) {
                cValueArray *path = check_and_cast<cValueArray*>(tree->get(k).objectValue());
                for (int l = 0; l < path->size(); l++) {
                    std::string name = path->get(l).stringValue();
                    auto pos = name.find('.');
                    auto nodeName = pos == std::string::npos ? name : name.substr(0, pos);
                    auto interfaceName = pos == std::string::npos ? "" : name.substr(pos + 1);
                    if (nodeName == networkNode->getFullName()) {
                        auto senderName = l != 0 ? path->get(l - 1).stringValue() : nullptr;
                        auto receiverName = l != path->size() - 1 ? path->get(l + 1).stringValue() : nullptr;
                        auto senderNetworkNodeName = senderName != nullptr ? getNodeName(senderName) : "";
                        auto receiverNetworkNodeName = receiverName != nullptr ? getNodeName(receiverName) : "";
                        if (senderName != nullptr && std::find(senderNetworkNodeNames.begin(), senderNetworkNodeNames.end(), senderNetworkNodeName) == senderNetworkNodeNames.end())
                            senderNetworkNodeNames.push_back(senderNetworkNodeName);
                        if (receiverName != nullptr && std::find(receiverNetworkNodeNames.begin(), receiverNetworkNodeNames.end(), receiverNetworkNodeName) == receiverNetworkNodeNames.end()) {
                            Interface *interface = nullptr;
                            if (interfaceName.empty())
                                interface = findLinkOut(node, receiverNetworkNodeName.c_str())->sourceInterface;
                            else
                                interface = *std::find_if(node->interfaces.begin(), node->interfaces.end(), [&] (auto interface) {
                                    return interface->networkInterface->getInterfaceName() == interfaceName;
                                });
                            receiverNetworkNodeNames.push_back(receiverNetworkNodeName);
                            interfaces.push_back(interface->networkInterface);
                        }
                    }
                }
            }
            stream.streamNodes[networkNodeName].receivers[j] = receiverNetworkNodeNames;
            stream.streamNodes[networkNodeName].interfaces[j] = interfaces;
        }
        stream.streamNodes[networkNodeName].senders = senderNetworkNodeNames;
    }
    for (auto& it : stream.streamNodes) {
        auto& streamNode = it.second;
        for (auto& r : streamNode.receivers) {
            if (!r.empty()) {
                if (std::find(streamNode.distinctReceivers.begin(), streamNode.distinctReceivers.end(), r) == streamNode.distinctReceivers.end())
                    streamNode.distinctReceivers.push_back(r);
            }
        }
    }
    EV_DEBUG << "Stream " << streamName << std::endl;
    for (auto& it : stream.streamNodes) {
        EV_DEBUG << "  Node " << it.first << std::endl;
        auto& streamNode = it.second;
        for (int i = 0; i < streamNode.senders.size(); i++) {
            auto& e = streamNode.senders[i];
            EV_DEBUG << "    Sender for tree " << i << " = " << e << std::endl;
        }
        for (int i = 0; i < streamNode.receivers.size(); i++) {
            auto& receiverNodes = streamNode.receivers[i];
            EV_DEBUG << "    Receivers for tree " << i << " = [";
            for (int j = 0; j < receiverNodes.size(); j++) {
                auto& e = receiverNodes[j];
                if (j != 0)
                    EV_DEBUG << ", ";
                EV_DEBUG << e;
            }
            EV_DEBUG << "]" << std::endl;
        }
    }
}

void MacForwardingTableConfigurator::computeStreamRules(cValueMap *streamConfiguration)
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        auto macForwardingTable = networkNode->findModuleByPath(".macTable");
        if (macForwardingTable != nullptr) {
            auto networkNodeName = networkNode->getFullName();
            int vlanId = streamConfiguration->containsKey("vlanId") ? atoi(streamConfiguration->get("vlanId")) : -1;
            std::string destinationAddress = streamConfiguration->containsKey("destinationAddress") ? streamConfiguration->get("destinationAddress") : streamConfiguration->get("destination");
            std::string streamName = streamConfiguration->get("name").stringValue();
            // encoding configuration
            auto& stream = streams[streamName];
            auto& streamNode = stream.streamNodes[networkNodeName];
            for (int j = 0; j < streamNode.receivers.size(); j++) {
                auto& receiverNetworkNodeNames = streamNode.receivers[j];
                if (!receiverNetworkNodeNames.empty()) {
                    std::string streamNameSuffix;
                    for (auto receiverNetworkNodeName : receiverNetworkNodeNames)
                        streamNameSuffix += "_" + receiverNetworkNodeName;
                    auto outputStreamName = streamNode.distinctReceivers.size() == 1 ? streamName : streamName + streamNameSuffix;
    //                auto it = std::find_if(node->streamEncodings.begin(), node->streamEncodings.end(), [&] (const StreamEncoding& streamEncoding) {
    //                    return streamEncoding.name == outputStreamName;
    //                });
    //                if (it != node->streamEncodings.end())
    //                    continue;
                    for (int k = 0; k < receiverNetworkNodeNames.size(); k++) {
                        auto receiverNetworkNodeName = receiverNetworkNodeNames[k];
                        auto networkInterface = streamNode.interfaces[j][k];
                        EV_DEBUG << "Adding rule" << EV_FIELD(streamName) << EV_FIELD(networkNodeName) << EV_FIELD(receiverNetworkNodeName) << EV_FIELD(destinationAddress) << EV_FIELD(vlanId) << EV_ENDL;
                        cValueMap *rule = new cValueMap();
                        rule->set("address", destinationAddress.c_str());
                        rule->set("vlan", vlanId);
                        rule->set("interface", networkInterface->getInterfaceName());
                        auto it = macForwardingTables.find(macForwardingTable->getId());
                        if (it == macForwardingTables.end()) {
                            cValueArray *rules = new cValueArray();
                            rules->add(rule);
                            macForwardingTables[macForwardingTable->getId()] = rules;
                        }
                        else
                            macForwardingTables[macForwardingTable->getId()]->add(rule);
                    }
                }
            }
        }
    }
}

cValueMap *MacForwardingTableConfigurator::computeStreamTree(cValueMap *configuration)
{
    auto streamName = configuration->get("name").stringValue();
    std::vector<std::string> destinations;
    auto sourceNetworkNodeName = configuration->get("source").stringValue();
    Node *sourceNode = static_cast<Node *>(topology->getNodeFor(getParentModule()->getSubmodule(sourceNetworkNodeName)));
    std::vector<const Node *> destinationNodes;
    cMatchExpression destinationFilter;
    destinationFilter.setPattern(configuration->get("destination").stringValue(), false, false, true);
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, node->module);
        if (destinationFilter.matches(&matchableObject)) {
            destinationNodes.push_back(node);
            destinations.push_back(node->module->getFullName());
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
    auto destinationAddress = configuration->containsKey("destinationAddress") ? configuration->get("destinationAddress").stringValue() : destinations[0];
    EV_INFO << "Collecting all possible trees" << EV_FIELD(streamName) << EV_ENDL;
    auto allTrees = collectAllTrees(sourceNode, destinationNodes);
    for (auto tree : allTrees)
        EV_INFO << "  Found tree" << EV_FIELD(streamName) << EV_FIELD(tree) << std::endl;
    EV_INFO << "Selecting best tree" << EV_FIELD(streamName) << EV_ENDL;
    auto tree = selectBestTree(configuration, sourceNode, destinationNodes, allTrees);
    EV_INFO << "Best tree having the best cost" << EV_FIELD(streamName) << EV_FIELD(sourceNodeName) << EV_FIELD(destinationNodeNames) << EV_ENDL;
    EV_INFO << "  Selected tree" << EV_FIELD(streamName) << EV_FIELD(tree) << std::endl;

    cValueMap *streamParameterValue = new cValueMap();
    cValueArray *treesParameterValue = new cValueArray();
    streamParameterValue->set("name", streamName);
    streamParameterValue->set("source", sourceNetworkNodeName);
    if (configuration->containsKey("vlanId"))
        streamParameterValue->set("vlanId", configuration->get("vlanId"));
    // TODO KLUDGE
    streamParameterValue->set("destination", destinations[0].c_str());
    streamParameterValue->set("destinationAddress", destinationAddress.c_str());
    cValueArray *treeParameterValue = new cValueArray();
    for (auto& path : tree.paths) {
        cValueArray *pathParameterValue = new cValueArray();
        for (auto& interface : path.interfaces) {
            std::string name;
            name = interface->node->module->getFullName();
            if (MacForwardingTableConfigurator::countParalellLinks(interface) > 1)
                name = name + "." + interface->networkInterface->getInterfaceName();
            pathParameterValue->add(name.c_str());
        }
        treeParameterValue->add(pathParameterValue);
    }
    treesParameterValue->add(treeParameterValue);
    streamParameterValue->set("trees", treesParameterValue);
    return streamParameterValue;
}

MacForwardingTableConfigurator::Tree MacForwardingTableConfigurator::selectBestTree(cValueMap *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const
{
    int bestTree = -1;
    double bestCost = DBL_MAX;
    for (int i = 0; i < trees.size(); i++) {
        double cost = computeTreeCost(sourceNode, destinationNodes, trees[i]);
        if (cost < bestCost) {
            bestCost = cost;
            bestTree = i;
        }
    }
    return trees[bestTree];
}

double MacForwardingTableConfigurator::computeTreeCost(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree) const
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

MacForwardingTableConfigurator::Tree MacForwardingTableConfigurator::computeCanonicalTree(const Tree& tree) const
{
    Tree canonicalTree;
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

std::vector<MacForwardingTableConfigurator::Tree> MacForwardingTableConfigurator::collectAllTrees(Node *sourceNode, const std::vector<const Node *>& destinationNodes) const
{
    std::vector<Tree> allTrees;
    topology->calculateUnweightedSingleShortestPathsTo(sourceNode);
    std::vector<Path> currentTree;
    std::vector<const Node *> stopNodes;
    stopNodes.push_back(sourceNode);
    collectAllTrees(stopNodes, destinationNodes, 0, currentTree, allTrees);
    return allTrees;
}

void MacForwardingTableConfigurator::collectAllTrees(const std::vector<const Node *>& stopNodes, const std::vector<const Node *>& destinationNodes, int destinationNodeIndex, std::vector<Path>& currentTree, std::vector<Tree>& allTrees) const
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

std::vector<MacForwardingTableConfigurator::Path> MacForwardingTableConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *destinationNode) const
{
    std::vector<Path> allPaths;
    std::vector<const Interface *> currentPath;
    collectAllPaths(stopNodes, destinationNode, currentPath, allPaths);
    return allPaths;
}

void MacForwardingTableConfigurator::collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *currentNode, std::vector<const Interface *>& currentPath, std::vector<Path>& allPaths) const
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

} // namespace inet

