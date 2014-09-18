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

#ifndef __INET_GENERICNETWORKCONFIGURATOR_H
#define __INET_GENERICNETWORKCONFIGURATOR_H

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/common/Topology.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

class PatternMatcher;

/**
 * This module configures generic routing tables for a network.
 *
 * For more info please see the NED file.
 */
class INET_API GenericNetworkConfigurator : public cSimpleModule
{
  public:
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
        GenericRoutingTable *routingTable;
        std::vector<InterfaceInfo *> interfaceInfos;

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
        bool configure;    // false means the IP address of the interface will not be modified
        L3Address address;    // the bits

        InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry)
        {
            this->node = node;
            this->linkInfo = linkInfo;
            this->interfaceEntry = interfaceEntry;
            configure = false;
        }

        virtual std::string getFullPath() const { return interfaceEntry->getFullPath(); }
    };

    /**
     * Represents a "link" in the network. A link is a communication medium between interfaces;
     * it can be e.g. a point-to-point link, an Ethernet LAN or a wireless LAN.
     */
    class LinkInfo : public cObject
    {
      public:
        bool isWireless;
        std::vector<InterfaceInfo *> interfaceInfos;    // interfaces on that LAN or point-to-point link
        InterfaceInfo *gatewayInterfaceInfo;    // non-NULL if all hosts have 1 non-loopback interface except one host that has two of them (this will be the gateway)

        LinkInfo(bool isWireless) { this->isWireless = isWireless; gatewayInterfaceInfo = NULL; }
        ~LinkInfo() { for (int i = 0; i < (int)interfaceInfos.size(); i++) delete interfaceInfos[i]; }
    };

    /**
     * Represents the network topology.
     */
    class GenericTopology : public Topology
    {
      public:
        std::vector<LinkInfo *> linkInfos;    // all links in the network

      public:
        virtual ~GenericTopology() { for (int i = 0; i < (int)linkInfos.size(); i++) delete linkInfos[i]; }

      protected:
        virtual Node *createNode(cModule *module) { return new GenericNetworkConfigurator::Node(module); }
        virtual Link *createLink() { return new GenericNetworkConfigurator::Link(); }
    };

    class Matcher
    {
      private:
        bool matchesany;
        std::vector<inet::PatternMatcher *> matchers;    // TODO replace with a MatchExpression once it becomes available in OMNeT++

      public:
        Matcher(const char *pattern);
        ~Matcher();
        bool matches(const char *s);
        bool matchesAny() { return matchesany; }
    };

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage);

    /**
     * Extracts network topology by walking through the module hierarchy.
     * Creates vertices from modules having @node property.
     * Creates edges from connections between network interfaces.
     */
    virtual void extractTopology(GenericTopology& topology);

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     * May add default routes and subnet routes if possible and requested.
     */
    virtual void addStaticRoutes(GenericTopology& topology);

    virtual void dumpTopology(GenericTopology& topology);
    virtual void dumpRoutes(GenericTopology& topology);

    // helper functions
    virtual void extractWiredTopology(GenericTopology& topology);
    virtual void extractWiredNeighbors(Topology::LinkOut *linkOut, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& nodesVisited);
    virtual void extractWirelessTopology(GenericTopology& topology);
    virtual InterfaceInfo *determineGatewayForLink(LinkInfo *linkInfo);
    virtual double getChannelWeight(cChannel *transmissionChannel);
    virtual bool isWirelessInterface(InterfaceEntry *interfaceEntry);
    virtual const char *getWirelessId(InterfaceEntry *interfaceEntry);

    virtual InterfaceInfo *createInterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry);
    virtual Topology::LinkOut *findLinkOut(Node *node, int gateId);
    virtual InterfaceInfo *findInterfaceInfo(Node *node, InterfaceEntry *interfaceEntry);
};

} // namespace inet

#endif // ifndef __INET_GENERICNETWORKCONFIGURATOR_H

