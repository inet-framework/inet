//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_L3NETWORKCONFIGURATORBASE_H
#define __INET_L3NETWORKCONFIGURATORBASE_H

#include <algorithm>

#include "inet/common/INETMath.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

class INET_API L3NetworkConfiguratorBase : public cSimpleModule, public L3AddressResolver
{
  public:
    class LinkInfo;
    class InterfaceInfo;

    /**
     * Represents a node in the network.
     */
    class INET_API Node : public inet::Topology::Node {
      public:
        cModule *module = nullptr;
        IInterfaceTable *interfaceTable = nullptr;
        IRoutingTable *routingTable = nullptr;
        std::vector<InterfaceInfo *> interfaceInfos;

      public:
        Node(cModule *module) : inet::Topology::Node(module->getId()), module(module) {}
        virtual ~Node() { for (auto& interfaceInfo : interfaceInfos) delete interfaceInfo; }
    };

    /**
     * Represents a connection (wired or wireless) in the network.
     */
    class INET_API Link : public inet::Topology::Link {
      public:
        InterfaceInfo *sourceInterfaceInfo = nullptr;
        InterfaceInfo *destinationInterfaceInfo = nullptr;

      public:
        Link() {}
    };

    /**
     * Represents an interface in the network.
     */
    class INET_API InterfaceInfo : public cObject {
      public:
        Node *node = nullptr;
        LinkInfo *linkInfo = nullptr;
        NetworkInterface *networkInterface = nullptr;
        int mtu = 0;
        double metric = NaN;
        bool configure = false; // false means the IP address of the interface will not be modified
        bool addStaticRoute = false; // add-static-route attribute
        bool addDefaultRoute = false; // add-default-route attribute
        bool addSubnetRoute = false; // add-subnet-route attribute

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface);

        virtual std::string getFullPath() const override { return networkInterface->getInterfaceFullPath(); }
    };

    /**
     * Represents a "link" in the network. A link is a communication medium between interfaces;
     * it can be e.g. a point-to-point link, an Ethernet LAN or a wireless LAN.
     */
    class INET_API LinkInfo : public cObject {
      public:
        std::vector<InterfaceInfo *> interfaceInfos; // interfaces on that LAN or point-to-point link
        InterfaceInfo *gatewayInterfaceInfo = nullptr; // non-nullptr if all hosts have 1 non-loopback interface except one host that has two of them (this will be the gateway)
        int networkId = 0;

      public:
        LinkInfo() {}
    };

    /**
     * Represents the network topology.
     */
    class INET_API Topology : public inet::Topology {
      public:
        std::vector<LinkInfo *> linkInfos; // all links in the network
        std::map<int, InterfaceInfo *> interfaceInfos; // all interfaces in the network

      public:
        virtual ~Topology() { for (auto& linkInfo : linkInfos) delete linkInfo; }

      protected:
        virtual Node *createNode(cModule *module) override { return new L3NetworkConfiguratorBase::Node(module); }
        virtual Link *createLink() override { return new L3NetworkConfiguratorBase::Link(); }
    };

    class INET_API Matcher {
      protected:
        bool matchesany = false;
        std::vector<inet::PatternMatcher *> matchers; // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();

        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

    class INET_API InterfaceMatcher {
      protected:
        bool matchesany = false;
        std::vector<inet::PatternMatcher *> nameMatchers;
        std::vector<inet::PatternMatcher *> towardsMatchers;

      public:
        InterfaceMatcher(const char *pattern);
        ~InterfaceMatcher();

        bool matches(InterfaceInfo *interfaceInfo);
        bool matchesAny() { return matchesany; }
    };

  protected:
    // parameters
    double minLinkWeight = NaN;
    bool configureIsolatedNetworksSeparatly = false;
    cXMLElement *configuration = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates vertices from modules having @networkNode property.
     * Creates edges from connections (wired and wireless) between network interfaces.
     */
    virtual void extractTopology(Topology& topology);

    // helper functions
    virtual void extractWiredNeighbors(Topology& topology, Topology::Link *linkOut, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractWirelessNeighbors(Topology& topology, const char *wirelessId, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractDeviceNeighbors(Topology& topology, Node *node, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited);
    virtual InterfaceInfo *determineGatewayForLink(LinkInfo *linkInfo);
    virtual double computeNodeWeight(Node *node, const char *metric, cXMLElement *parameters);
    virtual double computeLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual double computeWiredLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual double computeWirelessLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual bool isBridgeNode(Node *node);
    virtual std::string getWirelessId(NetworkInterface *networkInterface);
    virtual InterfaceInfo *createInterfaceInfo(Topology& topology, Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface);
    virtual Topology::Link *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, NetworkInterface *networkInterface);
    virtual IInterfaceTable *findInterfaceTable(Node *node);
    virtual IRoutingTable *findRoutingTable(Node *node);

    // generic helper functions
    virtual void dumpTopology(Topology& topology);
};

inline std::ostream& operator<<(std::ostream& stream, const L3NetworkConfiguratorBase::Link& link)
{
    return stream << (link.sourceInterfaceInfo != nullptr ? link.sourceInterfaceInfo->getFullPath() : "") << " -> "
                  << (link.destinationInterfaceInfo != nullptr ? link.destinationInterfaceInfo->getFullPath() : "");
}

inline std::ostream& operator<<(std::ostream& stream, const L3NetworkConfiguratorBase::Link *link)
{
    return stream << *link;
}

} // namespace inet

#endif

