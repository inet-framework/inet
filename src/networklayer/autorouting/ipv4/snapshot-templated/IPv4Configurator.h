//
// Copyright (C) 2011 Opensim Ltd
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

#ifndef __INET_IPV4CONFIGURATOR_H
#define __INET_IPV4CONFIGURATOR_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPv4Address.h"
#include "IPConfigurator.h"

namespace inet { class PatternMatcher; }


/**
 * Configures IPv4 addresses for a network.
 *
 * For more info please see the NED file.
 */
class INET_API IPv4Configurator : public IPConfigurator<uint32>
{
    public:
        /**
         * Simplified route representation used by the optimizer.
         * This class makes the optimization faster by introducing route coloring.
         */
        class RouteInfo {
            public:
                int color;          // an index into an array representing the different route actions (gateway, interface, metric, etc.)
                bool enabled;       // allows turning of routes without removing them from the list
                uint32 destination; // originally copied from the IPv4Route
                uint32 netmask;     // originally copied from the IPv4Route
                std::vector<RouteInfo *> originalRouteInfos; // routes that are routed by this one from the unoptimized original routing table, we keep track of this to be able to skip merge candidates with less computation

                RouteInfo(int color, uint32 destination, uint32 netmask) { this->color = color; this->enabled = true; this->destination = destination; this->netmask = netmask; }
        };

        /**
         * Simplified routing table representation used by the optimizer.
         */
        class RoutingTableInfo {
            public:
                std::vector<RouteInfo *> routeInfos; // list of routes in the routing table

                int addRouteInfo(RouteInfo *routeInfo) {
                    std::vector<RouteInfo *>::iterator it = upper_bound(routeInfos.begin(), routeInfos.end(), routeInfo, routeInfoLessThan);
                    int index = it - routeInfos.begin();
                    routeInfos.insert(it, routeInfo);
                    return index;
                }
                void removeRouteInfo(const RouteInfo *routeInfo) { routeInfos.erase(std::find(routeInfos.begin(), routeInfos.end(), routeInfo)); }
                RouteInfo *findBestMatchingRouteInfo(const uint32 destination) const { return findBestMatchingRouteInfo(destination, 0, routeInfos.size()); }
                RouteInfo *findBestMatchingRouteInfo(const uint32 destination, int begin, int end) const {
                    for (int index = begin; index < end; index++) {
                        RouteInfo *routeInfo = routeInfos.at(index);
                        if (routeInfo->enabled && !((destination ^ routeInfo->destination) & routeInfo->netmask))
                            return const_cast<RouteInfo *>(routeInfo);
                    }
                    return NULL;
                }
                static bool routeInfoLessThan(const RouteInfo *a, const RouteInfo *b) { return a->netmask != b->netmask ? a->netmask > b->netmask : a->destination < b->destination; }
        };

        class Matcher
        {
            private:
                bool matchesany;
                std::vector<inet::PatternMatcher*> matchers; // TODO replace with a MatchExpression once it becomes available in OMNeT++
            public:
                Matcher(const char *pattern);
                ~Matcher();
                bool matches(const char *s);
                bool matchesAny() { return matchesany; }
        };

    protected:
        // main functionality
        virtual void initialize(int stage);
        virtual void readAddressConfiguration(cXMLElement *root, Topology& topology, NetworkInfo& networkInfo);
        virtual void addManualRoutes(cXMLElement *root, Topology& topology, NetworkInfo& networkInfo);

        /**
         * Adds static routes to all routing tables in the network.
         * The algorithm uses Dijstra's weighted shortest path algorithm.
         * May add default routes and subnet routes if possible and requested.
         */
        virtual void addStaticRoutes(Topology& topology, NetworkInfo& networkInfo);

        /**
         * Destructively optimizes the given IPv4 routes by merging some of them.
         * The resulting routes might be different in that they will route packets
         * that the original routes did not. Nevertheless the following invariant
         * holds: any packet routed by the original routes will still be routed
         * the same way by the optimized routes.
         */
        virtual void optimizeRoutes(std::vector<IPv4Route *> *routes);

        virtual void dumpTopology(Topology& topology);
        virtual void dumpRoutes(Topology& topology);
        virtual void dumpConfig(Topology& topology, NetworkInfo& networkInfo);

        // helper functions
        virtual void assignAddress(InterfaceEntry *interfaceEntry, uint32 address, uint32 netmask);
        virtual std::string toString(uint32 value) { return IPv4Address(value).str(); }

        virtual void parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits);
        virtual bool linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule);
        virtual const char *getMandatoryAttribute(cXMLElement *element, const char *attr);
        virtual void resolveInterfaceAndGateway(NodeInfo *node, const char *interfaceAttr, const char *gatewayAttr, InterfaceEntry *&outIE, IPv4Address& outGateway, const NetworkInfo& networkInfo);
        virtual InterfaceInfo *findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node);
        virtual InterfaceInfo *findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, IPv4Address address);
        virtual LinkInfo *findLinkOfInterface(const NetworkInfo& networkInfo, InterfaceEntry *interfaceEntry);
        virtual InterfaceInfo *createInterfaceInfo(NodeInfo *nodeInfo, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);
};

#endif
