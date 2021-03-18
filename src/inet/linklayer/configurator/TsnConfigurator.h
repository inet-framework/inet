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

#ifndef __INET_TSNCONFIGURATOR_H
#define __INET_TSNCONFIGURATOR_H

#include <algorithm>
#include <vector>

#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

class INET_API TsnConfigurator : public cSimpleModule
{
  protected:
    class InterfaceInfo;

    class StreamConfiguration
    {
      public:
        std::string name;
        std::string packetFilter;
        std::string source;
        std::string destination;
        std::vector<std::vector<std::string>> paths;
    };

    /**
     * Represents a node in the network.
     */
    class Node : public Topology::Node {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        std::vector<InterfaceInfo *> interfaceInfos;

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
        virtual Node *createNode(cModule *module) override { return new TsnConfigurator::Node(module); }
        virtual Link *createLink() override { return new TsnConfigurator::Link(); }
    };

  protected:
    cValueArray *configuration;

    Topology topology;
    std::vector<StreamConfiguration> streamConfigurations;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
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

    virtual void computeStreams();
    virtual void computeStream(cValueMap *streamConfiguration);

    virtual std::vector<std::vector<std::string>> selectBestPathsSubset(cValueMap *configuration, const std::vector<std::vector<std::string>>& paths);
    virtual bool checkNodeFailureProtection(cValueArray *configuration, const std::vector<std::vector<std::string>>& paths);
    virtual bool checkLinkFailureProtection(cValueArray *configuration, const std::vector<std::vector<std::string>>& paths);

    virtual void configureStreams();

    virtual void collectAllPaths(Node *source, Node *destination, std::vector<std::vector<std::string>>& paths);
    virtual void collectAllPaths(Node *source, Node *destination, Node *node, std::vector<std::vector<std::string>>& paths, std::vector<std::string>& current);

    virtual std::vector<std::string> collectNetworkNodes(std::string filter);
    virtual std::vector<std::string> collectNetworkLinks(std::string filter);

    virtual bool matchesFilter(std::string name, std::string filter);
    virtual bool intersects(std::vector<std::string> list1, std::vector<std::string> list2);

    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, NetworkInterface *networkInterface);
};

} // namespace inet

#endif

