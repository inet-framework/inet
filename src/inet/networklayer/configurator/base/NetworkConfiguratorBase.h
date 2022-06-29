//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    class Interface;

    /**
     * Represents a node in the network.
     */
    class INET_API Node : public Topology::Node {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        IRoutingTable *routingTable = nullptr;
        std::vector<Interface *> interfaces;

      public:
        Node(cModule *module) : Topology::Node(module->getId()) { this->module = module; interfaceTable = nullptr; }
        ~Node() { for (size_t i = 0; i < interfaces.size(); i++) delete interfaces[i]; }
    };

    /**
     * Represents an interface in the network.
     */
    class INET_API Interface : public cObject {
      public:
        Node *node;
        NetworkInterface *networkInterface;

      public:
        Interface(Node *node, NetworkInterface *networkInterface) : node(node), networkInterface(networkInterface) {}
        virtual std::string getFullPath() const override { return networkInterface->getInterfaceFullPath(); }
    };

    class INET_API Link : public Topology::Link {
      public:
        Interface *sourceInterface;
        Interface *destinationInterface;

      public:
        Link() { sourceInterface = nullptr; destinationInterface = nullptr; }
    };

    class INET_API Topology : public inet::Topology {
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
    virtual Link *findLinkOut(const Interface *interface) const;
    virtual Topology::Link *findLinkOut(const Node *node, int gateId) const;
    virtual Interface *findInterface(const Node *node, NetworkInterface *networkInterface) const;
};

} // namespace inet

#endif

