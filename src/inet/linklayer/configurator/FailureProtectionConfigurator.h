//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FAILUREPROTECTIONCONFIGURATOR_H
#define __INET_FAILUREPROTECTIONCONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API FailureProtectionConfigurator : public NetworkConfiguratorBase
{
  public:
    class INET_API Path
    {
      public:
        std::vector<const Interface *> interfaces;

      public:
        Path(const std::vector<const Interface *>& interfaces) : interfaces(interfaces) { }

        friend std::ostream& operator<<(std::ostream& os, const FailureProtectionConfigurator::Path& path)
        {
            os << "[";
            for (int i = 0; i < path.interfaces.size(); i++) {
                auto interface = path.interfaces[i];
                if (i != 0)
                    os << ", ";
                os << interface->node->module->getFullName();
                if (FailureProtectionConfigurator::countParalellLinks(interface) > 1)
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
        Tree(const std::vector<Path>& paths) : paths(paths) { }

        friend std::ostream& operator<<(std::ostream& os, const FailureProtectionConfigurator::Tree& tree)
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

    class INET_API StreamConfiguration
    {
      public:
        std::string name;
        int pcp = -1;
        int gateIndex = -1;
        cValue packetFilter;
        std::string source;
        std::string destinationAddress;
        std::vector<std::string> destinations;
        std::vector<Tree> trees;
    };

  protected:
    cValueArray *configuration;

    std::vector<StreamConfiguration> streamConfigurations;

  public:
    const std::vector<StreamConfiguration>& getStreams() const { return streamConfigurations; }

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

  protected:
    virtual void initialize(int stage) override;

    /**
     * Computes the network configuration for all nodes in the network.
     * The result of the computation is only stored in the configurator.
     */
    virtual void computeConfiguration();

    virtual void computeStreams();
    virtual void computeStream(cValueMap *streamConfiguration);

    virtual std::vector<Tree> selectBestTreeSubset(cValueMap *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const;
    virtual double computeTreeCost(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree) const;
    virtual Tree computeCanonicalTree(const Tree& tree) const;
    virtual bool checkNodeFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const;
    virtual bool checkLinkFailureProtection(cValueArray *configuration, const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const std::vector<Tree>& trees) const;

    virtual void configureStreams() const;

    virtual std::vector<Tree> collectAllTrees(Node *sourceNode, const std::vector<const Node *>& destinationNodes) const;
    virtual void collectAllTrees(const std::vector<const Node *>& stopNodes, const std::vector<const Node *>& destinationNodes, int destinationNodeIndex, std::vector<Path>& currentTree, std::vector<Tree>& allTrees) const;

    virtual std::vector<Path> collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *destinationNode) const;
    virtual void collectAllPaths(const std::vector<const Node *>& stopNodes, const Node *currentNode, std::vector<const Interface *>& currentPath, std::vector<Path>& allPaths) const;

    virtual std::vector<const Node *> collectNetworkNodes(const std::string& filter) const;
    virtual std::vector<const Link *> collectNetworkLinks(const std::string& filter) const;

    virtual void collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Node *>& failedNodes, std::vector<bool>& reachedDestinationNodes) const;
    virtual void collectReachedNodes(const Node *sourceNode, const std::vector<const Node *>& destinationNodes, const Tree& tree, const std::vector<const Link *>& failedLinks, std::vector<bool>& reachedDestinationNodes) const;

    virtual bool matchesFilter(const std::string& name, const std::string& filter) const;
};

} // namespace inet

#endif

