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

#ifndef __INET_IPV4NETWORKCONFIGURATOR_H
#define __INET_IPV4NETWORKCONFIGURATOR_H

#include <algorithm>
#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/common/Topology.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/PatternMatcher.h"

namespace inet {

/**
 * This module provides the global static configuration for the IPv4RoutingTable and
 * the IPv4 network interfaces of all nodes in the network.
 *
 * For more info please see the NED file.
 */
// TODO: remove topology arguments from functions or perhaps move those functions into topology or leave it as it is?
class INET_API IPv4NetworkConfigurator : public cSimpleModule, public L3AddressResolver
{
  protected:
    class LinkInfo;
    class InterfaceInfo;

    /**
     * Represents a node in the network.
     */
    class Node : public Topology::Node
    {
      public:
        cModule *module;
        IInterfaceTable *interfaceTable;
        IIPv4RoutingTable *routingTable;
        std::vector<InterfaceInfo *> interfaceInfos;
        std::vector<IPv4Route *> staticRoutes;
        std::vector<IPv4MulticastRoute *> staticMulticastRoutes;

      public:
        Node(cModule *module) : Topology::Node(module->getId()) { this->module = module; interfaceTable = NULL; routingTable = NULL; }
    };

    /**
     * Represents a connection (wired or wireless) in the network.
     */
    class Link : public Topology::Link
    {
      public:
        InterfaceInfo *sourceInterfaceInfo;
        InterfaceInfo *destinationInterfaceInfo;

      public:
        Link() { sourceInterfaceInfo = NULL; destinationInterfaceInfo = NULL; }
    };

    /**
     * Represents an interface in the network.
     */
    class InterfaceInfo : public cObject
    {
      public:
        Node *node;
        LinkInfo *linkInfo;
        InterfaceEntry *interfaceEntry;
        int mtu;
        double metric;
        bool configure;    // false means the IP address of the interface will not be modified
        bool addStaticRoute;    // add-static-route attribute
        bool addDefaultRoute;    // add-default-route attribute
        bool addSubnetRoute;    // add-subnet-route attribute
        uint32 address;    // the bits
        uint32 addressSpecifiedBits;    // 1 means the bit is specified, 0 means the bit is unspecified
        uint32 netmask;    // the bits
        uint32 netmaskSpecifiedBits;    // 1 means the bit is specified, 0 means the bit is unspecified
        std::vector<IPv4Address> multicastGroups;

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);

        IPv4Address getAddress() const { ASSERT(addressSpecifiedBits == 0xFFFFFFFF); return IPv4Address(address); }
        IPv4Address getNetmask() const { ASSERT(netmaskSpecifiedBits == 0xFFFFFFFF); return IPv4Address(netmask); }
        virtual std::string getFullPath() const { return interfaceEntry->getFullPath(); }
    };

    /**
     * Represents a "link" in the network. A link is a communication medium between interfaces;
     * it can be e.g. a point-to-point link, an Ethernet LAN or a wireless LAN.
     */
    class LinkInfo : public cObject
    {
      public:
        std::vector<InterfaceInfo *> interfaceInfos;    // interfaces on that LAN or point-to-point link
        InterfaceInfo *gatewayInterfaceInfo;    // non-NULL if all hosts have 1 non-loopback interface except one host that has two of them (this will be the gateway)

      public:
        LinkInfo() { gatewayInterfaceInfo = NULL; }
        ~LinkInfo() { for (int i = 0; i < (int)interfaceInfos.size(); i++) delete interfaceInfos[i]; }
    };

    /**
     * Represents the network topology.
     */
    class IPv4Topology : public Topology
    {
      public:
        std::vector<LinkInfo *> linkInfos;    // all links in the network
        std::map<InterfaceEntry *, InterfaceInfo *> interfaceInfos;    // all interfaces in the network

      public:
        virtual ~IPv4Topology() { for (int i = 0; i < (int)linkInfos.size(); i++) delete linkInfos[i]; }

      protected:
        virtual Node *createNode(cModule *module) { return new IPv4NetworkConfigurator::Node(module); }
        virtual Link *createLink() { return new IPv4NetworkConfigurator::Link(); }
    };

    /**
     * Simplified route representation used by the optimizer.
     * This class makes the optimization faster by introducing route coloring.
     */
    class RouteInfo
    {
      public:
        int color;    // an index into an array representing the different route actions (gateway, interface, metric, etc.)
        bool enabled;    // allows turning of routes without removing them from the list
        uint32 destination;    // originally copied from the IPv4Route
        uint32 netmask;    // originally copied from the IPv4Route
        std::vector<RouteInfo *> originalRouteInfos;    // routes that are routed by this one from the unoptimized original routing table, we keep track of this to be able to skip merge candidates with less computation

      public:
        RouteInfo(int color, uint32 destination, uint32 netmask) { this->color = color; this->enabled = true; this->destination = destination; this->netmask = netmask; }
    };

    /**
     * Simplified routing table representation used by the optimizer.
     */
    class RoutingTableInfo
    {
      public:
        std::vector<RouteInfo *> routeInfos;    // list of routes in the routing table

      public:
        int addRouteInfo(RouteInfo *routeInfo);
        void removeRouteInfo(const RouteInfo *routeInfo) { routeInfos.erase(std::find(routeInfos.begin(), routeInfos.end(), routeInfo)); }
        RouteInfo *findBestMatchingRouteInfo(const uint32 destination) const { return findBestMatchingRouteInfo(destination, 0, routeInfos.size()); }
        RouteInfo *findBestMatchingRouteInfo(const uint32 destination, int begin, int end) const;
        static bool routeInfoLessThan(const RouteInfo *a, const RouteInfo *b) { return a->netmask != b->netmask ? a->netmask > b->netmask : a->destination < b->destination; }
    };

    class Matcher
    {
      protected:
        bool matchesany;
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
        bool matchesany;
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
    bool assignAddressesParameter;
    bool assignDisjunctSubnetAddressesParameter;
    bool addStaticRoutesParameter;
    bool addSubnetRoutesParameter;
    bool addDefaultRoutesParameter;
    bool optimizeRoutesParameter;
    cXMLElement *configuration;

    // internal state
    IPv4Topology topology;

  public:
    /**
     * Computes the IPv4 network configuration for all nodes in the network.
     * The result of the computation is only stored in the network configurator.
     */
    virtual void computeConfiguration();

    /**
     * Configures all interfaces in the network based on the current network configuration.
     */
    virtual void configureAllInterfaces();

    /**
     * Configures the provided interface based on the current network configuration.
     */
    virtual void configureInterface(InterfaceEntry *interfaceEntry);

    /**
     * Configures all routing tables in the network based on the current network configuration.
     */
    virtual void configureAllRoutingTables();

    /**
     * Configures the provided routing table based on the current network configuration.
     */
    virtual void configureRoutingTable(IIPv4RoutingTable *routingTable);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage);

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates vertices from modules having @node property.
     * Creates edges from connections (wired and wireless) between network interfaces.
     */
    virtual void extractTopology(IPv4Topology& topology);

    /**
     * Reads interface elements from the configuration file and stores result.
     */
    virtual void readInterfaceConfiguration(IPv4Topology& topology);

    /**
     * Reads multicast-group elements from the configuration file and stores the result
     */
    virtual void readMulticastGroupConfiguration(IPv4Topology& topology);

    /**
     * Reads route elements from configuration file and stores the result
     */
    virtual void readManualRouteConfiguration(IPv4Topology& topology);

    /**
     * Reads multicast-route elements from configuration file and stores the result.
     */
    virtual void readManualMulticastRouteConfiguration(IPv4Topology& topology);

    /**
     * Assigns the addresses for all interfaces based on the parameters given
     * in the configuration file. See the NED file for details.
     */
    virtual void assignAddresses(IPv4Topology& topology);

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     * May add default routes and subnet routes if possible and requested.
     */
    virtual void addStaticRoutes(IPv4Topology& topology);

    /**
     * Destructively optimizes the given IPv4 routes by merging some of them.
     * The resulting routes might be different in that they will route packets
     * that the original routes did not. Nevertheless the following invariant
     * holds: any packet routed by the original routes will still be routed
     * the same way by the optimized routes.
     */
    virtual void optimizeRoutes(std::vector<IPv4Route *>& routes);

    void ensureConfigurationComputed(IPv4Topology& topology);
    void configureInterface(InterfaceInfo *interfaceInfo);
    void configureRoutingTable(Node *node);

    /**
     * Prints the current network configuration to the module output.
     */
    virtual void dumpConfiguration();
    virtual void dumpTopology(IPv4Topology& topology);
    virtual void dumpLinks(IPv4Topology& topology);
    virtual void dumpAddresses(IPv4Topology& topology);
    virtual void dumpRoutes(IPv4Topology& topology);
    virtual void dumpConfig(IPv4Topology& topology);

    // helper functions
    virtual void extractWiredNeighbors(IPv4Topology& topology, Topology::LinkOut *linkOut, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractWirelessNeighbors(IPv4Topology& topology, const char *wirelessId, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractDeviceNeighbors(IPv4Topology& topology, Node *node, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited);
    virtual InterfaceInfo *determineGatewayForLink(LinkInfo *linkInfo);
    virtual double getChannelWeight(cChannel *transmissionChannel);
    virtual bool isBridgeNode(Node *node);
    virtual bool isWirelessInterface(InterfaceEntry *interfaceEntry);
    virtual const char *getWirelessId(InterfaceEntry *interfaceEntry);
    virtual InterfaceInfo *createInterfaceInfo(IPv4Topology& topology, Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);
    virtual void parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits);
    virtual bool linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule);
    virtual const char *getMandatoryAttribute(cXMLElement *element, const char *attr);
    virtual void resolveInterfaceAndGateway(Node *node, const char *interfaceAttr, const char *gatewayAttr, InterfaceEntry *& outIE, IPv4Address& outGateway, IPv4Topology& topology);
    virtual InterfaceInfo *findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node);
    virtual InterfaceInfo *findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, IPv4Address address);
    virtual LinkInfo *findLinkOfInterface(IPv4Topology& topology, InterfaceEntry *interfaceEntry);
    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, InterfaceEntry *interfaceEntry);

    // helpers for address assignment
    static bool compareInterfaceInfos(InterfaceInfo *i, InterfaceInfo *j);
    void collectCompatibleInterfaces(const std::vector<InterfaceInfo *>& interfaces,    /*in*/
            std::vector<InterfaceInfo *>& compatibleInterfaces,    /*out, and the rest too*/
            uint32& mergedAddress, uint32& mergedAddressSpecifiedBits, uint32& mergedAddressIncompatibleBits,
            uint32& mergedNetmask, uint32& mergedNetmaskSpecifiedBits, uint32& mergedNetmaskIncompatibleBits);

    // helpers for routing table optimization
    bool containsRoute(const std::vector<IPv4Route *>& routes, IPv4Route *route);
    bool routesHaveSameColor(IPv4Route *route1, IPv4Route *route2);
    int findRouteIndexWithSameColor(const std::vector<IPv4Route *>& routes, IPv4Route *route);
    bool routesCanBeSwapped(RouteInfo *routeInfo1, RouteInfo *routeInfo2);
    bool routesCanBeNeighbors(const std::vector<RouteInfo *>& routeInfos, int i, int j);
    bool interruptsOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, RouteInfo *originalRouteInfo);
    bool interruptsAnyOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos);
    bool interruptsSubsequentOriginalRoutes(const RoutingTableInfo& routingTableInfo, int index);
    void checkOriginalRoutes(const RoutingTableInfo& routingTableInfo, const std::vector<RouteInfo *>& originalRouteInfos);
    void findLongestCommonDestinationPrefix(uint32 destination1, uint32 netmask1, uint32 destination2, uint32 netmask2, uint32& destinationOut, uint32& netmaskOut);
    void addOriginalRouteInfos(RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos);
    bool tryToMergeTwoRoutes(RoutingTableInfo& routingTableInfo, int i, int j, RouteInfo *routeInfoI, RouteInfo *routeInfoJ);
    bool tryToMergeAnyTwoRoutes(RoutingTableInfo& routingTableInfo);

    // address resolver interface
    bool getInterfaceIPv4Address(L3Address& ret, InterfaceEntry *interfaceEntry, bool netmask);
};

} // namespace inet

#endif // ifndef __INET_IPV4NETWORKCONFIGURATOR_H

