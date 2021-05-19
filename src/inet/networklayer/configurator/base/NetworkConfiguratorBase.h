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

#ifndef __INET_NETWORKCONFIGURATORBASE_H
#define __INET_NETWORKCONFIGURATORBASE_H

#include "inet/common/Topology.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

class INET_API NetworkConfiguratorBase : public cSimpleModule
{
  protected:
    class InterfaceInfo;

    /**
     * Represents a node in the network.
     */
    class Node : public Topology::Node {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        IRoutingTable *routingTable = nullptr;
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
        Node *node;
        NetworkInterface *networkInterface;

      public:
        InterfaceInfo(Node *node, NetworkInterface *networkInterface) : node(node), networkInterface(networkInterface) {}
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
        virtual Node *createNode(cModule *module) override { return new NetworkConfiguratorBase::Node(module); }
        virtual Link *createLink() override { return new NetworkConfiguratorBase::Link(); }
    };

  protected:
    Topology *topology = nullptr;

  protected:
    virtual ~NetworkConfiguratorBase() { delete topology; }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("Invalid operation"); }

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates nodes from modules having @networkNode property.
     * Creates links from connections (wired and wireless) between network interfaces.
     */
    virtual void extractTopology(Topology& topology);

    virtual std::vector<Node *> computeShortestNodePath(Node *source, Node *destination) const;
    virtual std::vector<Link *> computeShortestLinkPath(Node *source, Node *destination) const;

    virtual bool isBridgeNode(Node *node) const;

    virtual Link *findLinkIn(const Node *node, const char *neighbor) const;
    virtual Link *findLinkOut(const Node *node, const char *neighbor) const;
    virtual Link *findLinkOut(const Node *node, const Node *neighbor) const;
    virtual Link *findLinkOut(const InterfaceInfo *interfaceInfo) const;
    virtual Topology::LinkOut *findLinkOut(const Node *node, int gateId) const;
    virtual InterfaceInfo *findInterfaceInfo(const Node *node, NetworkInterface *networkInterface) const;
};

} // namespace inet

#endif

