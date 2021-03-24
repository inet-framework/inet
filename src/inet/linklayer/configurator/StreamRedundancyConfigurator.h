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

#ifndef __INET_STREAMREDUNDANCYCONFIGURATOR_H
#define __INET_STREAMREDUNDANCYCONFIGURATOR_H

#include <algorithm>
#include <vector>

#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API StreamRedundancyConfigurator : public cSimpleModule
{
  protected:
    class InterfaceInfo;

    class StreamIdentification
    {
      public:
        std::string stream;
        std::string packetFilter;
    };

    class StreamDecoding
    {
      public:
        NetworkInterface *networkInterface = nullptr;
        int vlanId = -1;
        std::string name;
    };

    class StreamMerging
    {
      public:
        std::vector<std::string> inputStreams;
        std::string outputStream;
    };

    class StreamSplitting
    {
      public:
        std::string inputStream;
        std::vector<std::string> outputStreams;
    };

    class StreamEncoding
    {
      public:
        std::string name;
        NetworkInterface *networkInterface = nullptr;
        std::string destination;
        int vlanId = -1;
    };

    /**
     * Represents a node in the network.
     */
    class Node : public Topology::Node {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        std::vector<InterfaceInfo *> interfaceInfos;
        std::vector<StreamIdentification> streamIdentifications;
        std::vector<StreamEncoding> streamEncodings;
        std::vector<StreamDecoding> streamDecodings;
        std::vector<StreamMerging> streamMergings;
        std::vector<StreamSplitting> streamSplittings;

      public:
        Node(cModule *module) : Topology::Node(module->getId()) { this->module = module; interfaceTable = nullptr; }
        ~Node() { for (size_t i = 0; i < interfaceInfos.size(); i++) delete interfaceInfos[i]; }
    };

    /**
     * Represents an interface in the network.
     */
    class InterfaceInfo : public cObject {
      public:
        NetworkInterface *networkInterface;

      public:
        InterfaceInfo(NetworkInterface *networkInterface) : networkInterface(networkInterface) {}
        virtual std::string getFullPath() const override { return networkInterface->getInterfaceFullPath(); }
    };

    class Link : public Topology::Link {
      public:
        InterfaceInfo *sourceInterfaceInfo;
        InterfaceInfo *destinationInterfaceInfo;

      public:
        Link() { sourceInterfaceInfo = nullptr; destinationInterfaceInfo = nullptr; }
    };

    class Topology : public inet::Topology {
      protected:
        virtual Node *createNode(cModule *module) override { return new StreamRedundancyConfigurator::Node(module); }
        virtual Link *createLink() override { return new StreamRedundancyConfigurator::Link(); }
    };

  protected:
    int minVlanId = -1;
    int maxVlanId = -1;
    cValueArray *configuration;

    Topology topology;
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> streamSenders; // maps network node name and stream name to list of previous sender network node names
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> receivers; // maps network node name and stream name to list of next receiver network node names
    std::map<std::pair<std::string, std::string>, int> nextVlanIds; // maps network node name and destination node name to next available VLAN ID
    std::map<std::tuple<std::string, std::string, std::string, std::string>, int> assignedVlanIds; // maps network node name, receiver network node name, destination network node name and stream name to VLAN ID

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates vertices from modules having @networkNode property.
     * Creates edges from connections (wired and wireless) between network interfaces.
     */
    virtual void extractTopology(Topology& topology);

    /**
     * Computes the network configuration for all nodes in the network.
     * The result of the computation is only stored in the configurator.
     */
    virtual void computeConfiguration();
    virtual void clearConfiguration();

    virtual void computeStreams();
    virtual void computeStreamSendersAndReceivers(cValueMap *streamConfiguration);
    virtual void computeStreamEncodings(cValueMap *streamConfiguration);
    virtual void computeStreamPolicyConfigurations(cValueMap *streamConfiguration);

    virtual void configureStreams();
    virtual void configureStreams(Node *node);

    virtual Link *findLinkIn(Node *node, const char *neighbor);
    virtual Link *findLinkOut(Node *node, const char *neighbor);
    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, NetworkInterface *networkInterface);

  public:
    virtual std::vector<std::vector<std::string>> getPathFragments(const char *stream);
};

} // namespace inet

#endif

