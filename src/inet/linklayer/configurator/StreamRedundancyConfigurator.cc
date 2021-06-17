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

#include "inet/linklayer/configurator/StreamRedundancyConfigurator.h"

#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(StreamRedundancyConfigurator);

void StreamRedundancyConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        minVlanId = par("minVlanId");
        maxVlanId = par("maxVlanId");
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        computeConfiguration();
        configureStreams();
    }
}

void StreamRedundancyConfigurator::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "configuration")) {
            configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
            clearConfiguration();
            computeConfiguration();
            configureStreams();
        }
    }
}

void StreamRedundancyConfigurator::extractTopology(Topology& topology)
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

void StreamRedundancyConfigurator::clearConfiguration()
{
    streamSenders.clear();
    receivers.clear();
    nextVlanIds.clear();
    assignedVlanIds.clear();
    topology.clear();
}

void StreamRedundancyConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    TIME(extractTopology(topology));
    TIME(computeStreams());
    printElapsedTime("initialize", initializeStartTime);
}

void StreamRedundancyConfigurator::computeStreams()
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        computeStreamSendersAndReceivers(streamConfiguration);
        computeStreamEncodings(streamConfiguration);
        computeStreamPolicyConfigurations(streamConfiguration);
    }
}

void StreamRedundancyConfigurator::computeStreamSendersAndReceivers(cValueMap *streamConfiguration)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        cValueArray *paths = check_and_cast<cValueArray *>(streamConfiguration->get("paths").objectValue());
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        std::vector<std::string> senderNetworkNodeNames;
        std::vector<std::string> receiverNetworkNodeNames;
        for (int j = 0; j < paths->size(); j++) {
            cValueArray *path = check_and_cast<cValueArray*>(paths->get(j).objectValue());
            for (int k = 0; k < path->size() + 2; k++) {
                auto getElement = [&] (int i) {
                    return i == 0 ? sourceNetworkNodeName.c_str() : i == path->size() + 1 ? destinationNetworkNodeName.c_str() : path->get(i - 1).stringValue();
                };
                const char *elementNetworkNodeName = getElement(k);
                if (!strcmp(elementNetworkNodeName, networkNode->getFullName())) {
                    auto senderNetworkNodeName = k != 0 ? getElement(k - 1) : nullptr;
                    auto receiverNetworkNodeName = k != path->size() + 1 ? getElement(k + 1) : nullptr;
                    if (senderNetworkNodeName != nullptr && !contains(senderNetworkNodeNames, senderNetworkNodeName))
                        senderNetworkNodeNames.push_back(senderNetworkNodeName);
                    if (receiverNetworkNodeName != nullptr && !contains(receiverNetworkNodeNames, receiverNetworkNodeName))
                        receiverNetworkNodeNames.push_back(receiverNetworkNodeName);
                }
            }
        }
        std::string streamName = streamConfiguration->get("name").stringValue();
        streamSenders[{networkNodeName, streamName}] = senderNetworkNodeNames;
        receivers[{networkNodeName, streamName}] = receiverNetworkNodeNames;
    }
}

void StreamRedundancyConfigurator::computeStreamEncodings(cValueMap *streamConfiguration)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        std::string streamName = streamConfiguration->get("name").stringValue();
        std::vector<std::string> senderNetworkNodeNames = streamSenders[{networkNodeName, streamName}];
        std::vector<std::string> receiverNetworkNodeNames = receivers[{networkNodeName, streamName}];
        // encoding configuration
        for (auto receiverNetworkNodeName : receiverNetworkNodeNames) {
            auto outputStreamName = receiverNetworkNodeNames.size() == 1 ? streamName : streamName + "_" + receiverNetworkNodeName;
            auto linkOut = findLinkOut(node, receiverNetworkNodeName.c_str());
            auto it = nextVlanIds.emplace(std::pair<std::string, std::string>{networkNodeName, destinationNetworkNodeName}, 0);
            int vlanId = it.first->second++;
            if (vlanId > maxVlanId)
                throw cRuntimeError("Cannot assign VLAN ID in the available range");
            assignedVlanIds[{networkNodeName, receiverNetworkNodeName, destinationNetworkNodeName, streamName}] = vlanId;
            StreamEncoding streamEncoding;
            streamEncoding.name = outputStreamName;
            streamEncoding.networkInterface = linkOut->sourceInterfaceInfo->networkInterface;
            streamEncoding.vlanId = vlanId;
            streamEncoding.destination = destinationNetworkNodeName;
            node->streamEncodings.push_back(streamEncoding);
        }
    }
}

void StreamRedundancyConfigurator::computeStreamPolicyConfigurations(cValueMap *streamConfiguration)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        std::string streamName = streamConfiguration->get("name").stringValue();
        std::vector<std::string> senderNetworkNodeNames = streamSenders[{networkNodeName, streamName}];
        std::vector<std::string> receiverNetworkNodeNames = receivers[{networkNodeName, streamName}];
        // identification configuration
        if (networkNodeName == sourceNetworkNodeName) {
            StreamIdentification streamIdentification;
            streamIdentification.stream = streamConfiguration->get("name").stringValue();
            streamIdentification.packetFilter = streamConfiguration->get("packetFilter").stringValue();
            node->streamIdentifications.push_back(streamIdentification);
        }
        // decoding configuration
        for (auto senderNetworkNodeName : senderNetworkNodeNames) {
            auto inputStreamName = senderNetworkNodeNames.size() == 1 ? streamName : streamName + "_" + senderNetworkNodeName;
            auto linkIn = findLinkIn(node, senderNetworkNodeName.c_str());
            auto vlanId = assignedVlanIds[{senderNetworkNodeName, networkNodeName, destinationNetworkNodeName, streamName}];
            StreamDecoding streamDecoding;
            streamDecoding.name = inputStreamName;
            streamDecoding.networkInterface = linkIn->destinationInterfaceInfo->networkInterface;
            streamDecoding.vlanId = vlanId;
            if (streamDecoding.vlanId > maxVlanId)
                throw cRuntimeError("Cannot assign VLAN ID in the available range");
            node->streamDecodings.push_back(streamDecoding);
        }
        // merging configuration
        if (senderNetworkNodeNames.size() > 1) {
            StreamMerging streamMerging;
            streamMerging.outputStream = streamName;
            for (auto senderNetworkNodeName : senderNetworkNodeNames) {
                auto inputStreamName = streamName + "_" + senderNetworkNodeName;
                streamMerging.inputStreams.push_back(inputStreamName);
            }
            node->streamMergings.push_back(streamMerging);
        }
        // splitting configuration
        if (receiverNetworkNodeNames.size() > 1) {
            StreamSplitting streamSplitting;
            streamSplitting.inputStream = streamName;
            for (auto receiverNetworkNodeName : receiverNetworkNodeNames) {
                auto outputStreamName = streamName + "_" + receiverNetworkNodeName;
                streamSplitting.outputStreams.push_back(outputStreamName);
            }
            node->streamSplittings.push_back(streamSplitting);
        }
    }
}

void StreamRedundancyConfigurator::configureStreams()
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        auto node = (Node *)topology.getNode(i);
        configureStreams(node);
    }
}

void StreamRedundancyConfigurator::configureStreams(Node *node)
{
    auto networkNode = node->module;
    auto macAddressTable = networkNode->findModuleByPath(".macTable");
    auto ieee8021qTagHeaderChecker = networkNode->findModuleByPath(".ieee8021q.qTagHeaderChecker");
    auto streamPolicy = networkNode->findModuleByPath(".bridging.streamPolicy");
    if (streamPolicy == nullptr)
        streamPolicy = networkNode->findModuleByPath(".ieee8021r.policy");
    auto streamIdentifier = streamPolicy->getSubmodule("streamIdentifier");
    auto streamDecoder = streamPolicy->getSubmodule("streamDecoder");
    auto streamMerger = streamPolicy->getSubmodule("streamMerger");
    auto streamSplitter = streamPolicy->getSubmodule("streamSplitter");
    auto streamEncoder = streamPolicy->getSubmodule("streamEncoder");
    if (streamIdentifier != nullptr && !node->streamIdentifications.empty()) {
        cValueArray *parameterValue = new cValueArray();
        for (auto& streamIdentification : node->streamIdentifications) {
            cValueMap *value = new cValueMap();
            value->set("packetFilter", streamIdentification.packetFilter);
            value->set("stream", streamIdentification.stream);
            parameterValue->add(value);
        }
        EV_INFO << "Configuring stream merging" << EV_FIELD(networkNode) << EV_FIELD(streamIdentifier) << EV_FIELD(parameterValue) << EV_ENDL;
        streamIdentifier->par("streamMappings") = parameterValue;
    }
    if (streamDecoder != nullptr && !node->streamDecodings.empty()) {
        cValueArray *parameterValue = new cValueArray();
        for (auto& streamDecoding : node->streamDecodings) {
            cValueMap *value = new cValueMap();
            value->set("interface", streamDecoding.networkInterface->getInterfaceName());
            value->set("vlan", streamDecoding.vlanId);
            value->set("stream", streamDecoding.name.c_str());
            parameterValue->add(value);
        }
        EV_INFO << "Configuring stream decoding" << EV_FIELD(networkNode) << EV_FIELD(streamDecoder) << EV_FIELD(parameterValue) << EV_ENDL;
        streamDecoder->par("streamMappings") = parameterValue;
    }
    if (streamMerger != nullptr && !node->streamMergings.empty()) {
        cValueMap *parameterValue = new cValueMap();
        for (auto& streamMerging : node->streamMergings) {
            for (auto inputStream : streamMerging.inputStreams)
                parameterValue->set(inputStream.c_str(), streamMerging.outputStream.c_str());
        }
        EV_INFO << "Configuring stream merging" << EV_FIELD(networkNode) << EV_FIELD(streamMerger) << EV_FIELD(parameterValue) << EV_ENDL;
        streamMerger->par("streamMapping") = parameterValue;
    }
    if (streamSplitter != nullptr && !node->streamSplittings.empty()) {
        cValueMap *parameterValue = new cValueMap();
        for (auto& streamSplitting : node->streamSplittings) {
            cValueArray *value = new cValueArray();
            for (auto outputStream : streamSplitting.outputStreams)
                value->add(outputStream.c_str());
            parameterValue->set(streamSplitting.inputStream.c_str(), value);
        }
        EV_INFO << "Configuring stream splitting" << EV_FIELD(networkNode) << EV_FIELD(streamSplitter) << EV_FIELD(parameterValue) << EV_ENDL;
        streamSplitter->par("streamMapping") = parameterValue;
    }
    if (streamEncoder != nullptr && !node->streamEncodings.empty()) {
        cValueMap *parameterValue = new cValueMap();
        for (auto& streamEncoding : node->streamEncodings)
            parameterValue->set(streamEncoding.name.c_str(), streamEncoding.vlanId);
        EV_INFO << "Configuring stream encoding" << EV_FIELD(networkNode) << EV_FIELD(streamEncoder) << EV_FIELD(parameterValue) << EV_ENDL;
        streamEncoder->par("streamNameToVlanIdMapping") = parameterValue;
    }
    if (macAddressTable != nullptr && !node->streamEncodings.empty()) {
        cValueArray *parameterValue = new cValueArray();
        for (auto& streamEncoding : node->streamEncodings) {
            cValueMap *value = new cValueMap();
            value->set("address", streamEncoding.destination.c_str());
            value->set("vlan", streamEncoding.vlanId);
            value->set("interface", streamEncoding.networkInterface->getInterfaceName());
            parameterValue->add(value);
        }
        EV_INFO << "Configuring MAC address table" << EV_FIELD(networkNode) << EV_FIELD(macAddressTable) << EV_FIELD(parameterValue) << EV_ENDL;
        macAddressTable->par("addressTable") = parameterValue;
    }
    if (ieee8021qTagHeaderChecker != nullptr) {
        std::set<int> vlanIds;
        for (auto& streamDecoding : node->streamDecodings)
            vlanIds.insert(streamDecoding.vlanId);
        cValueArray *parameterValue = new cValueArray();
        for (int vlanId : vlanIds)
            parameterValue->add(vlanId);
        EV_INFO << "Configuring VLAN filter" << EV_FIELD(networkNode) << EV_FIELD(ieee8021qTagHeaderChecker) << EV_FIELD(parameterValue) << EV_ENDL;
        ieee8021qTagHeaderChecker->par("vlanIdFilter") = parameterValue;
    }
}

std::vector<std::vector<std::string>> StreamRedundancyConfigurator::getPathFragments(const char *stream)
{
    for (int i = 0; i < configuration->size(); i++) {
        cValueMap *streamConfiguration = check_and_cast<cValueMap *>(configuration->get(i).objectValue());
        if (!strcmp(streamConfiguration->get("name").stringValue(), stream)) {
            std::vector<std::vector<std::string>> memberStreams;
            auto source = streamConfiguration->get("source").stringValue();
            auto destination = streamConfiguration->get("destination").stringValue();
            std::string streamName = streamConfiguration->get("name").stringValue();
            cValueArray *paths = check_and_cast<cValueArray *>(streamConfiguration->get("paths").objectValue());
            for (int j = 0; j < paths->size(); j++) {
                std::vector<std::string> memberStream;
                memberStream.push_back(source);
                cValueArray *path = check_and_cast<cValueArray*>(paths->get(j).objectValue());
                for (int k = 0; k < path->size(); k++) {
                    auto nodeName = path->get(k).stringValue();
                    auto module = getParentModule()->getSubmodule(nodeName);
                    Node *node = (Node *)topology.getNodeFor(module);
                    bool isMerging = false;
                    for (auto streamMerging : node->streamMergings)
                        if (streamMerging.outputStream == streamName)
                            isMerging = true;
                    bool isSplitting = false;
                    for (auto streamSplitting : node->streamSplittings)
                        if (streamSplitting.inputStream == streamName)
                            isSplitting = true;
                    memberStream.push_back(nodeName);
                    if (isMerging || isSplitting) {
                        if (!memberStream.empty() && !contains(memberStreams, memberStream))
                            memberStreams.push_back(memberStream);
                        memberStream.clear();
                        memberStream.push_back(nodeName);
                    }
                }
                memberStream.push_back(destination);
                if (!memberStream.empty() && !contains(memberStreams, memberStream))
                    memberStreams.push_back(memberStream);
            }
            return memberStreams;
        }
    }
    throw cRuntimeError("Stream not found");
}

StreamRedundancyConfigurator::Link *StreamRedundancyConfigurator::findLinkIn(Node *node, const char *neighbor)
{
    for (int i = 0; i < node->getNumInLinks(); i++)
        if (!strcmp(node->getLinkIn(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkIn(i)));
    return nullptr;
}

StreamRedundancyConfigurator::Link *StreamRedundancyConfigurator::findLinkOut(Node *node, const char *neighbor)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (!strcmp(node->getLinkOut(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkOut(i)));
    return nullptr;
}

Topology::LinkOut *StreamRedundancyConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return nullptr;
}

StreamRedundancyConfigurator::InterfaceInfo *StreamRedundancyConfigurator::findInterfaceInfo(Node *node, NetworkInterface *networkInterface)
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interfaceInfo : node->interfaceInfos)
        if (interfaceInfo->networkInterface == networkInterface)
            return interfaceInfo;

    return nullptr;
}

} // namespace inet

