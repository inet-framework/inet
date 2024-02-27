//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMREDUNDANCYCONFIGURATOR_H
#define __INET_STREAMREDUNDANCYCONFIGURATOR_H

#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

namespace inet {

class INET_API StreamRedundancyConfigurator : public NetworkConfiguratorBase
{
  protected:
    class INET_API StreamIdentification
    {
      public:
        std::string stream;
        cValue packetFilter;
    };

    class INET_API StreamDecoding
    {
      public:
        NetworkInterface *networkInterface = nullptr;
        int vlanId = -1;
        std::string name;
    };

    class INET_API StreamMerging
    {
      public:
        std::vector<std::string> inputStreams;
        std::string outputStream;
    };

    class INET_API StreamSplitting
    {
      public:
        std::string inputStream;
        std::vector<std::string> outputStreams;
    };

    class INET_API StreamEncoding
    {
      public:
        std::string name;
        NetworkInterface *networkInterface = nullptr;
        std::string destination;
        int vlanId = -1;
        int pcp = -1;
    };

    /**
     * Represents a node in the network.
     */
    class INET_API Node : public NetworkConfiguratorBase::Node {
      public:
        std::vector<StreamIdentification> streamIdentifications;
        std::vector<StreamEncoding> streamEncodings;
        std::vector<StreamDecoding> streamDecodings;
        std::vector<StreamMerging> streamMergings;
        std::vector<StreamSplitting> streamSplittings;

      public:
        Node(cModule *module) : NetworkConfiguratorBase::Node(module) { }
    };

    class INET_API Topology : public NetworkConfiguratorBase::Topology {
      protected:
        virtual Node *createNode(cModule *module) override { return new StreamRedundancyConfigurator::Node(module); }
    };

    // TODO use this, see below
    class INET_API StreamNodeTreeData
    {
      public:
        std::string sender;
        std::vector<std::string> receivers;
        std::vector<std::string> distinctReceivers;
    };

    class INET_API StreamNode
    {
      public:
        // TODO use this, see above
        std::vector<StreamNodeTreeData> treeData;

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
    int minVlanId = -1;
    int maxVlanId = -1;
    cValueArray *configuration;

    std::map<std::string, Stream> streams;
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
    virtual std::vector<std::string> getStreamNames();
    virtual std::vector<std::vector<std::string>> getPathFragments(const char *stream);
};

} // namespace inet

#endif

