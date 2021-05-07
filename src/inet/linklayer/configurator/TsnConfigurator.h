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
    class StreamConfiguration
    {
      public:
        std::string name;
        std::string packetFilter;
        std::string source;
        std::string destination;
        std::vector<std::vector<std::string>> paths;
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
};

} // namespace inet

#endif

