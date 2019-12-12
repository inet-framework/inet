//
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//
// Authors: Levente Meszaros (primary author), Andras Varga, Tamas Borbely
//

#ifndef __INET_NETWORKCONFIGURATORBASE_H
#define __INET_NETWORKCONFIGURATORBASE_H

#include <algorithm>

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/Topology.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

// TODO: move to some utility file
inline bool isEmpty(const char *s) { return !s || !s[0]; }
inline bool isNotEmpty(const char *s) { return s && s[0]; }

class INET_API NetworkConfiguratorBase : public cSimpleModule, public L3AddressResolver
{
  public:
    class LinkInfo;
    class InterfaceInfo;

    /**
     * Represents a node in the network.
     */
    class Node : public inet::Topology::Node
    {
      public:
        cModule *module = nullptr;
        IInterfaceTable *interfaceTable = nullptr;
        IRoutingTable *routingTable = nullptr;
        std::vector<InterfaceInfo *> interfaceInfos;

      public:
        Node(cModule *module) : inet::Topology::Node(module->getId()), module(module) { }
        virtual ~Node() { for (auto & interfaceInfo : interfaceInfos) delete interfaceInfo; }
    };

    /**
     * Represents a connection (wired or wireless) in the network.
     */
    class Link : public inet::Topology::Link
    {
      public:
        InterfaceInfo *sourceInterfaceInfo = nullptr;
        InterfaceInfo *destinationInterfaceInfo = nullptr;

      public:
        Link() { }
    };

    /**
     * Represents an interface in the network.
     */
    class InterfaceInfo : public cObject
    {
      public:
        Node *node = nullptr;
        LinkInfo *linkInfo = nullptr;
        InterfaceEntry *interfaceEntry = nullptr;
        int mtu = 0;
        double metric = NaN;
        bool configure = false;    // false means the IP address of the interface will not be modified
        bool addStaticRoute = false;    // add-static-route attribute
        bool addDefaultRoute = false;    // add-default-route attribute
        bool addSubnetRoute = false;    // add-subnet-route attribute

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);

        virtual std::string getFullPath() const override { return interfaceEntry->getInterfaceFullPath(); }
    };

    /**
     * Represents a "link" in the network. A link is a communication medium between interfaces;
     * it can be e.g. a point-to-point link, an Ethernet LAN or a wireless LAN.
     */
    class LinkInfo : public cObject
    {
      public:
        std::vector<InterfaceInfo *> interfaceInfos;    // interfaces on that LAN or point-to-point link
        InterfaceInfo *gatewayInterfaceInfo = nullptr;    // non-NULL if all hosts have 1 non-loopback interface except one host that has two of them (this will be the gateway)
        int networkId = 0;

      public:
        LinkInfo() { }
    };

    /**
     * Represents the network topology.
     */
    class Topology : public inet::Topology
    {
      public:
        std::vector<LinkInfo *> linkInfos;    // all links in the network
        std::map<int, InterfaceInfo *> interfaceInfos;    // all interfaces in the network

      public:
        virtual ~Topology() { for (auto & linkInfo : linkInfos) delete linkInfo; }

      protected:
        virtual Node *createNode(cModule *module) override { return new NetworkConfiguratorBase::Node(module); }
        virtual Link *createLink() override { return new NetworkConfiguratorBase::Link(); }
    };

    class Matcher
    {
      protected:
        bool matchesany = false;
        std::vector<inet::PatternMatcher *> matchers;    // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();

        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

    class InterfaceMatcher
    {
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
    virtual void extractWiredNeighbors(Topology& topology, Topology::LinkOut *linkOut, LinkInfo *linkInfo, std::map<int, InterfaceEntry *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractWirelessNeighbors(Topology& topology, const char *wirelessId, LinkInfo *linkInfo, std::map<int, InterfaceEntry *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractDeviceNeighbors(Topology& topology, Node *node, LinkInfo *linkInfo, std::map<int, InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited);
    virtual InterfaceInfo *determineGatewayForLink(LinkInfo *linkInfo);
    virtual double computeNodeWeight(Node *node, const char *metric, cXMLElement *parameters);
    virtual double computeLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual double computeWiredLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual double computeWirelessLinkWeight(Link *link, const char *metric, cXMLElement *parameters);
    virtual bool isBridgeNode(Node *node);
    virtual bool isWirelessInterface(InterfaceEntry *interfaceEntry);
    virtual std::string getWirelessId(InterfaceEntry *interfaceEntry);
    virtual InterfaceInfo *createInterfaceInfo(Topology& topology, Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);
    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, InterfaceEntry *interfaceEntry);
    virtual IInterfaceTable *findInterfaceTable(Node *node);
    virtual IRoutingTable *findRoutingTable(Node *node);

    // generic helper functions
    virtual void dumpTopology(Topology& topology);
};

inline std::ostream& operator<<(std::ostream& stream, const NetworkConfiguratorBase::Link& link)
{
    return stream << (link.sourceInterfaceInfo != nullptr ? link.sourceInterfaceInfo->getFullPath() : "") << " -> "
                  << (link.destinationInterfaceInfo != nullptr ? link.destinationInterfaceInfo->getFullPath() : "");
}

inline std::ostream& operator<<(std::ostream& stream, const NetworkConfiguratorBase::Link *link)
{
    return stream << *link;
}

} // namespace inet

#endif // ifndef __INET_NETWORKCONFIGURATORBASE_H

