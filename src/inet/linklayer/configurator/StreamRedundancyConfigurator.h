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

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API StreamRedundancyConfigurator : public NetworkConfiguratorBase
{
  protected:
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
    class Node : public NetworkConfiguratorBase::Node {
      public:
        std::vector<StreamIdentification> streamIdentifications;
        std::vector<StreamEncoding> streamEncodings;
        std::vector<StreamDecoding> streamDecodings;
        std::vector<StreamMerging> streamMergings;
        std::vector<StreamSplitting> streamSplittings;

      public:
        Node(cModule *module) : NetworkConfiguratorBase::Node(module) { }
    };

    class Topology : public NetworkConfiguratorBase::Topology {
      protected:
        virtual Node *createNode(cModule *module) override { return new StreamRedundancyConfigurator::Node(module); }
    };

  protected:
    int minVlanId = -1;
    int maxVlanId = -1;
    cValueArray *configuration;

    std::map<std::tuple<std::string, std::string, int>, std::vector<std::string>> senders; // maps network node name and stream name to list of previous sender network node names
    std::map<std::tuple<std::string, std::string, int>, std::vector<std::string>> receivers; // maps network node name and stream name to list of next receiver network node names
    std::map<std::pair<std::string, std::string>, int> nextVlanIds; // maps network node name and destination node name to next available VLAN ID
    std::map<std::tuple<std::string, std::string, std::string, std::string>, int> assignedVlanIds; // maps network node name, receiver network node name, destination network node name and stream name to VLAN ID

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

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

  public:
    virtual std::vector<std::vector<std::string>> getPathFragments(const char *stream);
};

} // namespace inet

#endif

