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

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API TsnConfigurator : public NetworkConfiguratorBase
{
  public:
    class Path
    {
      public:
        std::vector<const Interface *> interfaces;

      public:
        Path(const std::vector<const Interface *>& interfaces) : interfaces(interfaces) { }

        friend std::ostream& operator<<(std::ostream& os, const TsnConfigurator::Path& path)
        {
            os << "[";
            for (int i = 0; i < path.interfaces.size(); i++) {
                auto interface = path.interfaces[i];
                if (i != 0)
                    os << ", ";
                os << interface->node->module->getFullName();
                if (TsnConfigurator::countParalellLinks(interface) > 1)
                    os << "." << interface->networkInterface->getInterfaceName();
            }
            os << "]";
            return os;
        }
    };

    class Tree
    {
      public:
        std::vector<Path> paths;

      public:
        Tree(const std::vector<Path>& paths) : paths(paths) { }

        friend std::ostream& operator<<(std::ostream& os, const TsnConfigurator::Tree& tree)
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

    class StreamConfiguration
    {
      public:
        std::string name;
        std::string packetFilter;
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

