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

namespace inet {

Define_Module(StreamRedundancyConfigurator);

void StreamRedundancyConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
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

void StreamRedundancyConfigurator::clearConfiguration()
{
    senders.clear();
    receivers.clear();
    nextVlanIds.clear();
    assignedVlanIds.clear();
    if (topology != nullptr)
        topology->clear();
}

void StreamRedundancyConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    delete topology;
    topology = new Topology();
    TIME(extractTopology(*topology));
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
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        cValueArray *trees = check_and_cast<cValueArray *>(streamConfiguration->get("trees").objectValue());
        for (int j = 0; j < trees->size(); j++) {
            std::vector<std::string> senderNetworkNodeNames;
            std::vector<std::string> receiverNetworkNodeNames;
            cValueArray *tree = check_and_cast<cValueArray*>(trees->get(j).objectValue());
            for (int k = 0; k < tree->size(); k++) {
                cValueArray *path = check_and_cast<cValueArray*>(tree->get(k).objectValue());
                for (int l = 0; l < path->size(); l++) {
                    const char *elementNetworkNodeName = path->get(l).stringValue();
                    if (!strcmp(elementNetworkNodeName, networkNode->getFullName())) {
                        auto senderNetworkNodeName = l != 0 ? path->get(l - 1).stringValue() : nullptr;
                        auto receiverNetworkNodeName = l != path->size() - 1 ? path->get(l + 1).stringValue() : nullptr;
                        if (senderNetworkNodeName != nullptr && std::find(senderNetworkNodeNames.begin(), senderNetworkNodeNames.end(), senderNetworkNodeName) == senderNetworkNodeNames.end())
                            senderNetworkNodeNames.push_back(senderNetworkNodeName);
                        if (receiverNetworkNodeName != nullptr && std::find(receiverNetworkNodeNames.begin(), receiverNetworkNodeNames.end(), receiverNetworkNodeName) == receiverNetworkNodeNames.end())
                            receiverNetworkNodeNames.push_back(receiverNetworkNodeName);
                    }
                }
            }
            std::string streamName = streamConfiguration->get("name").stringValue();
            senders[{networkNodeName, streamName, j}] = senderNetworkNodeNames;
            receivers[{networkNodeName, streamName, j}] = receiverNetworkNodeNames;
        }
    }
}

void StreamRedundancyConfigurator::computeStreamEncodings(cValueMap *streamConfiguration)
{
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        std::string streamName = streamConfiguration->get("name").stringValue();
        std::vector<std::string> senderNetworkNodeNames;
        std::vector<std::string> receiverNetworkNodeNames;
        cValueArray *trees = check_and_cast<cValueArray *>(streamConfiguration->get("trees").objectValue());
        for (int j = 0; j < trees->size(); j++) {
            for (auto sender : senders[{networkNodeName, streamName, j}])
                if (std::find(senderNetworkNodeNames.begin(), senderNetworkNodeNames.end(), sender) == senderNetworkNodeNames.end())
                    senderNetworkNodeNames.push_back(sender);
            for (auto receiver : receivers[{networkNodeName, streamName, j}])
                if (std::find(receiverNetworkNodeNames.begin(), receiverNetworkNodeNames.end(), receiver) == receiverNetworkNodeNames.end())
                    receiverNetworkNodeNames.push_back(receiver);
        }
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
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
        auto networkNode = node->module;
        auto networkNodeName = networkNode->getFullName();
        std::string sourceNetworkNodeName = streamConfiguration->get("source");
        std::string destinationNetworkNodeName = streamConfiguration->get("destination");
        std::string streamName = streamConfiguration->get("name").stringValue();
        std::vector<std::string> senderNetworkNodeNames;
        std::vector<std::string> receiverNetworkNodeNames;
        cValueArray *trees = check_and_cast<cValueArray *>(streamConfiguration->get("trees").objectValue());
        for (int j = 0; j < trees->size(); j++) {
            for (auto sender : senders[{networkNodeName, streamName, j}])
                if (std::find(senderNetworkNodeNames.begin(), senderNetworkNodeNames.end(), sender) == senderNetworkNodeNames.end())
                    senderNetworkNodeNames.push_back(sender);
            for (auto receiver : receivers[{networkNodeName, streamName, j}])
                if (std::find(receiverNetworkNodeNames.begin(), receiverNetworkNodeNames.end(), receiver) == receiverNetworkNodeNames.end())
                    receiverNetworkNodeNames.push_back(receiver);
        }
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
    for (int i = 0; i < topology->getNumNodes(); i++) {
        auto node = (Node *)topology->getNode(i);
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
            cValueArray *trees = check_and_cast<cValueArray *>(streamConfiguration->get("trees").objectValue());
            for (int j = 0; j < trees->size(); j++) {
                cValueArray *tree = check_and_cast<cValueArray*>(trees->get(j).objectValue());
                for (int k = 0; k < tree->size(); k++) {
                    cValueArray *path = check_and_cast<cValueArray*>(tree->get(k).objectValue());
                    std::vector<std::string> memberStream;
                    memberStream.push_back(source);
                    for (int l = 1; l < path->size() - 1; l++) {
                        auto nodeName = path->get(l).stringValue();
                        auto module = getParentModule()->getSubmodule(nodeName);
                        Node *node = (Node *)topology->getNodeFor(module);
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
                            if (!memberStream.empty() && std::find(memberStreams.begin(), memberStreams.end(), memberStream) == memberStreams.end())
                                memberStreams.push_back(memberStream);
                            memberStream.clear();
                            memberStream.push_back(nodeName);
                        }
                    }
                    memberStream.push_back(destination);
                    if (!memberStream.empty() && std::find(memberStreams.begin(), memberStreams.end(), memberStream) == memberStreams.end())
                        memberStreams.push_back(memberStream);
                }
            }
            return memberStreams;
        }
    }
    throw cRuntimeError("Stream not found");
}

} // namespace inet

