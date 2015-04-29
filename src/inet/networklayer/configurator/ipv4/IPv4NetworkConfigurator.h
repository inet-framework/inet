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
#include "inet/common/Topology.h"
#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

/**
 * This module provides the global static configuration for the IPv4RoutingTable and
 * the IPv4 network interfaces of all nodes in the network.
 *
 * For more info please see the NED file.
 */
class INET_API IPv4NetworkConfigurator : public NetworkConfiguratorBase
{
  protected:
    /**
     * Represents a node in the network.
     */
    class Node : public NetworkConfiguratorBase::Node
    {
      public:
        std::vector<IPv4Route *> staticRoutes;
        std::vector<IPv4MulticastRoute *> staticMulticastRoutes;

      public:
        Node(cModule *module) : NetworkConfiguratorBase::Node(module) { }
        ~Node() {
            for (int i = 0; i < (int)staticRoutes.size(); i++) delete staticRoutes[i];
            for (int i = 0; i < (int)staticMulticastRoutes.size(); i++) delete staticMulticastRoutes[i];
        }
    };

    class Topology : public NetworkConfiguratorBase::Topology
    {
      protected:
        virtual Node *createNode(cModule *module) override { return new IPv4NetworkConfigurator::Node(module); }
    };

    /**
     * Represents an interface in the network.
     */
    class InterfaceInfo : public NetworkConfiguratorBase::InterfaceInfo
    {
      public:
        uint32 address;    // the bits
        uint32 addressSpecifiedBits;    // 1 means the bit is specified, 0 means the bit is unspecified
        uint32 netmask;    // the bits
        uint32 netmaskSpecifiedBits;    // 1 means the bit is specified, 0 means the bit is unspecified
        std::vector<IPv4Address> multicastGroups;

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);

        IPv4Address getAddress() const { ASSERT(addressSpecifiedBits == 0xFFFFFFFF); return IPv4Address(address); }
        IPv4Address getNetmask() const { ASSERT(netmaskSpecifiedBits == 0xFFFFFFFF); return IPv4Address(netmask); }
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
        ~RouteInfo() {} // don't delete originalRouteInfos elements, they are not exclusively owned
    };

    /**
     * Simplified routing table representation used by the optimizer.
     */
    class RoutingTableInfo
    {
      public:
        std::vector<RouteInfo *> originalRouteInfos;    // keep track of the original routes
        std::vector<RouteInfo *> routeInfos;    // list of routes in the routing table

      public:
        RoutingTableInfo() {}
        ~RoutingTableInfo() { for (int i = 0; i < (int)originalRouteInfos.size(); i++) delete originalRouteInfos[i]; }

        int addRouteInfo(RouteInfo *routeInfo);
        void removeRouteInfo(const RouteInfo *routeInfo) { routeInfos.erase(std::find(routeInfos.begin(), routeInfos.end(), routeInfo)); }
        RouteInfo *findBestMatchingRouteInfo(const uint32 destination) const { return findBestMatchingRouteInfo(destination, 0, routeInfos.size()); }
        RouteInfo *findBestMatchingRouteInfo(const uint32 destination, int begin, int end) const;
        static bool routeInfoLessThan(const RouteInfo *a, const RouteInfo *b) { return a->netmask != b->netmask ? a->netmask > b->netmask : a->destination < b->destination; }
    };

  protected:
    // parameters
    bool assignAddressesParameter;
    bool assignDisjunctSubnetAddressesParameter;
    bool addStaticRoutesParameter;
    bool addSubnetRoutesParameter;
    bool addDefaultRoutesParameter;
    bool optimizeRoutesParameter;

    // internal state
    Topology topology;

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
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;

    /**
     * Reads interface elements from the configuration file and stores result.
     */
    virtual void readInterfaceConfiguration(Topology& topology);

    /**
     * Reads multicast-group elements from the configuration file and stores the result
     */
    virtual void readMulticastGroupConfiguration(Topology& topology);

    /**
     * Reads route elements from configuration file and stores the result
     */
    virtual void readManualRouteConfiguration(Topology& topology);

    /**
     * Reads multicast-route elements from configuration file and stores the result.
     */
    virtual void readManualMulticastRouteConfiguration(Topology& topology);

    /**
     * Assigns the addresses for all interfaces based on the parameters given
     * in the configuration file. See the NED file for details.
     */
    virtual void assignAddresses(Topology& topology);

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     * May add default routes and subnet routes if possible and requested.
     */
    virtual void addStaticRoutes(Topology& topology, cXMLElement *element);

    /**
     * Destructively optimizes the given IPv4 routes by merging some of them.
     * The resulting routes might be different in that they will route packets
     * that the original routes did not. Nevertheless the following invariant
     * holds: any packet routed by the original routes will still be routed
     * the same way by the optimized routes.
     */
    virtual void optimizeRoutes(std::vector<IPv4Route *>& routes);

    void ensureConfigurationComputed(Topology& topology);
    void configureInterface(InterfaceInfo *interfaceInfo);
    void configureRoutingTable(Node *node);

    /**
     * Prints the current network configuration to the module output.
     */
    virtual void dumpConfiguration();
    virtual void dumpLinks(Topology& topology);
    virtual void dumpAddresses(Topology& topology);
    virtual void dumpRoutes(Topology& topology);
    virtual void dumpConfig(Topology& topology);

    // helper functions
    virtual InterfaceInfo *createInterfaceInfo(NetworkConfiguratorBase::Topology& topology, NetworkConfiguratorBase::Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry) override;
    virtual void parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits);
    virtual bool linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule);
    virtual const char *getMandatoryAttribute(cXMLElement *element, const char *attr);
    virtual void resolveInterfaceAndGateway(Node *node, const char *interfaceAttr, const char *gatewayAttr, InterfaceEntry *& outIE, IPv4Address& outGateway, Topology& topology);
    virtual InterfaceInfo *findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node);
    virtual InterfaceInfo *findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, IPv4Address address);
    virtual LinkInfo *findLinkOfInterface(Topology& topology, InterfaceEntry *interfaceEntry);
    virtual IRoutingTable *findRoutingTable(NetworkConfiguratorBase::Node *node) override;

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
    bool getInterfaceIPv4Address(L3Address& ret, InterfaceEntry *interfaceEntry, bool netmask) override;
};

} // namespace inet

#endif // ifndef __INET_IPV4NETWORKCONFIGURATOR_H

