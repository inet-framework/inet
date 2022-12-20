//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MACFORWARDINGTABLECONFIGURATOR_H
#define __INET_MACFORWARDINGTABLECONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API MacForwardingTableConfigurator : public NetworkConfiguratorBase, public cListener
{
  protected:
    class INET_API Path
    {
      public:
        std::vector<const Interface *> interfaces;

      public:
        Path(const std::vector<const Interface *>& interfaces) : interfaces(interfaces) { }

        friend std::ostream& operator<<(std::ostream& os, const MacForwardingTableConfigurator::Path& path)
        {
            os << "[";
            for (int i = 0; i < path.interfaces.size(); i++) {
                auto interface = path.interfaces[i];
                if (i != 0)
                    os << ", ";
                os << interface->node->module->getFullName();
                if (MacForwardingTableConfigurator::countParalellLinks(interface) > 1)
                    os << "." << interface->networkInterface->getInterfaceName();
            }
            os << "]";
            return os;
        }
    };

    class INET_API Tree
    {
      public:
        std::vector<Path> paths;

      public:
        Tree() { }
        Tree(const std::vector<Path>& paths) : paths(paths) { }

        friend std::ostream& operator<<(std::ostream& os, const MacForwardingTableConfigurator::Tree& tree)
        {
            os << "{";
            for (int i = 0; i < tree.paths.size(); i++) {
                auto path = tree.paths[i];
                if (i != 0)
                    os << ", ";
                os << path;
            }
            os << "}";
            return os;
        }
    };

    class INET_API StreamNode
    {
      public:
        std::vector<std::string> senders; // tree index to previous sender network node name
        std::vector<std::vector<NetworkInterface *>> interfaces; // tree index to outgoing interface name
        std::vector<std::vector<std::string>> receivers; // tree index to list of next receiver network node names
        std::vector<std::vector<std::string>> distinctReceivers; // distinct non-empty receiver sets
    };

    class INET_API Stream
    {
      public:
        std::map<std::string, StreamNode> streamNodes;
    };

  protected:
    cValueArray *configuration;
    std::map<std::string, Stream> streams;
    std::map<int, cValueArray *> macForwardingTables;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Computes the network configuration for all nodes in the network.
     * The result of the computation is only stored in the configurator.
     */
    virtual void computeConfiguration();
    virtual void extendConfiguration(Node *destinationNode, Interface *destinationInterface, MacAddress macAddress);

    virtual void computeUnicastRules();
    virtual void configureMacForwardingTables() const;

    virtual cValueMap *findForwardingRule(cValueArray *configuration, MacAddress macAddress, std::string interfaceName);

    virtual void computeStreamRules();
    virtual cValueMap *computeStreamTree(cValueMap *streamConfiguration);
    virtual void computeStreamSendersAndReceivers(cValueMap *streamConfiguration);
    virtual void computeStreamRules(cValueMap *streamConfiguration);

    // TODO: the following is redundant with FailureProtectionConfigurator
    virtual Tree selectBestTree(cValueMap *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const;
    virtual double computeTreeCost(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree) const;
    virtual Tree computeCanonicalTree(const Tree& tree) const;

    virtual std::vector<Tree> collectAllTrees(Node *sourceNode, const std::vector<const Node *>& destinationNodes) const;
    virtual void collectAllTrees(const std::vector<const Node *>& stopNodes, const std::vector<const Node *>& destinationNodes, int destinationNodeIndex, std::vector<Path>& currentTree, std::vector<Tree>& allTrees) const;

    virtual std::vector<Path> collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *destinationNode) const;
    virtual void collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *currentNode, std::vector<const Interface *>& currentPath, std::vector<Path>& allPaths) const;

    static std::string getNodeName(std::string name) {
        auto pos = name.find('.');
        return pos == std::string::npos ? name : name.substr(0, pos);
    }

    static Node *findConnectedNode(const Interface *interface) {
        auto node = interface->node;
        for (int i = 0; i < node->getNumOutLinks(); i++) {
            auto link = (Link *)node->getLinkOut(i);
            if (link->sourceInterface == interface)
                return link->destinationInterface->node;
        }
        return nullptr;
    }

    static int countParalellLinks(const Interface *interface) {
        int count = 0;
        auto node = interface->node;
        for (auto otherInterface : node->interfaces)
            if (findConnectedNode(interface) == findConnectedNode(otherInterface))
                count++;
        return count;
    }

  public:
    virtual ~MacForwardingTableConfigurator();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

