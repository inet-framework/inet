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
  protected:
    class Path
    {
      public:
        std::vector<std::string> nodes;

      public:
        Path(const std::vector<std::string>& nodes) : nodes(nodes) { }
    };

    class Tree
    {
      public:
        std::vector<Path> paths;

      public:
        Tree(const std::vector<Path>& paths) : paths(paths) { }
    };

    class StreamConfiguration
    {
      public:
        std::string name;
        std::string packetFilter;
        std::string source;
        std::string destination;
        std::vector<Tree> trees;
    };

  protected:
    cValueArray *configuration;

    std::vector<StreamConfiguration> streamConfigurations;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Computes the network configuration for all nodes in the network.
     * The result of the computation is only stored in the configurator.
     */
    virtual void computeConfiguration();

    virtual void computeStreams();
    virtual void computeStream(cValueMap *streamConfiguration);

    virtual std::vector<Tree> selectBestTreeSubset(cValueMap *configuration, const std::vector<Tree>& trees);
    virtual bool checkNodeFailureProtection(cValueArray *configuration, const std::vector<Tree>& trees);
    virtual bool checkLinkFailureProtection(cValueArray *configuration, const std::vector<Tree>& trees);

    virtual void configureStreams();

    virtual void collectAllTrees(Node *source, Node *destination, std::vector<Tree>& trees);
    virtual void collectAllTrees(Node *source, std::vector<Node *> destinations, std::vector<Tree>& trees);
    virtual void collectAllTrees(Node *source, std::vector<Node *> destinations, Node *node, std::vector<Tree>& trees, std::vector<std::string>& current);

    virtual std::vector<std::string> collectNetworkNodes(std::string filter);
    virtual std::vector<std::string> collectNetworkLinks(std::string filter);

    virtual bool matchesFilter(std::string name, std::string filter);
    virtual bool intersects(std::vector<std::string> list1, std::vector<std::string> list2);
};

} // namespace inet

#endif

