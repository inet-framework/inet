//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6NETWORKCONFIGURATOR_H
#define __INET_IPV6NETWORKCONFIGURATOR_H

#include "inet/common/Topology.h"
#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

/**
 * This module provides the global static configuration for the Ipv6RoutingTable and
 * the Ipv6 network interfaces of all nodes in the network.
 *
 * For more info please see the NED file.
 */
class INET_API Ipv6NetworkConfigurator : public L3NetworkConfiguratorBase, public cListener
{
  public:
    static simsignal_t networkConfigurationChangedSignal;

  protected:
    /**
     * Represents a node in the network.
     */
    class INET_API Node : public L3NetworkConfiguratorBase::Node {
      public:
        std::vector<Ipv6Route *> staticRoutes;
        std::vector<Ipv6MulticastRoute *> staticMulticastRoutes;
        std::vector<const NetworkInterface *> routingTableNetworkInterfaces;

      public:
        Node(cModule *module) : L3NetworkConfiguratorBase::Node(module) {}
        ~Node() {
            for (size_t i = 0; i < staticRoutes.size(); i++) delete staticRoutes[i];
            for (size_t i = 0; i < staticMulticastRoutes.size(); i++) delete staticMulticastRoutes[i];
        }
    };

    class INET_API Topology : public L3NetworkConfiguratorBase::Topology {
      protected:
        virtual Node *createNode(cModule *module) override { return new Ipv6NetworkConfigurator::Node(module); }
    };

    /**
     * Represents an interface in the network.
     */
    class INET_API InterfaceInfo : public L3NetworkConfiguratorBase::InterfaceInfo {
      public:
        Ipv6Address prefix;            // assigned /64 prefix for this interface's link
        int prefixLength = 64;         // prefix length (typically 64)
        uint32_t specifiedGroups = 0;  // bitmask: bit i (from MSB) = 1 means 16-bit group i is specified in the prefix template
        Ipv6Address explicitAddress;   // full address from the @address attribute; unspecified = not set (derive the IID from the interface token)
        Ipv6Address globalAddress;     // the full global address (prefix + IID)

        // RA/NDP parameters from XML (sentinel -1 = not specified, use module default)
        int advValidLifetime = -1;      // AdvPrefix valid lifetime in seconds
        int advPreferredLifetime = -1;  // AdvPrefix preferred lifetime in seconds
        int advOnLinkFlag = -1;         // AdvPrefix on-link flag (0/1, -1 = default)
        int advAutonomousFlag = -1;     // AdvPrefix autonomous flag (0/1, -1 = default)
        int advManagedFlag = -1;        // M flag in RA (0/1, -1 = default)
        int advOtherConfigFlag = -1;    // O flag in RA (0/1, -1 = default)
        double maxRtrAdvInterval = -1;  // max interval between RAs in seconds
        double minRtrAdvInterval = -1;  // min interval between RAs in seconds
        int advDefaultLifetime = -1;    // Router Lifetime in RA in seconds
        int dupAddrDetectTransmits = -1; // number of DAD NS transmissions

      public:
        InterfaceInfo(Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface);
    };

  protected:
    // parameters
    bool assignAddressesParameter = false;
    bool assignAddressesToHostsParameter = true;
    bool addStaticRoutesParameter = false;
    bool addDefaultRoutesParameter = false;
    bool addDirectRoutesParameter = false;

    // internal state
    Topology topology;

  public:
    /**
     * Computes the Ipv6 network configuration for all nodes in the network.
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
    virtual void configureInterface(NetworkInterface *networkInterface);

    /**
     * Configures all routing tables in the network based on the current network configuration.
     */
    virtual void configureAllRoutingTables();

    /**
     * Configures the provided routing table based on the current network configuration.
     */
    virtual void configureRoutingTable(Ipv6RoutingTable *routingTable);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;

    /**
     * Reads interface elements from the configuration file and stores result.
     */
    virtual void readInterfaceConfiguration(Topology& topology);

    /**
     * Reads route elements from configuration file and stores the result.
     */
    virtual void readManualRouteConfiguration(Topology& topology);

    /**
     * Reads multicast-route elements from the configuration file and stores the
     * result. Each <multicast-route> names an RPF (parent) interface and a set of
     * output (children) interfaces, and is installed as a static Ipv6MulticastRoute.
     */
    virtual void readManualMulticastRouteConfiguration(Topology& topology);

    /**
     * Assigns prefixes and global addresses to all interfaces.
     */
    virtual void assignAddresses(Topology& topology);

    /**
     * Adds static routes to all routing tables in the network.
     * The algorithm uses Dijkstra's weighted shortest path algorithm.
     */
    virtual void addStaticRoutes(Topology& topology, cXMLElement *autorouteElement);

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

    // helper functions
    virtual InterfaceInfo *createInterfaceInfo(L3NetworkConfiguratorBase::Topology& topology, L3NetworkConfiguratorBase::Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface) override;
    virtual IRoutingTable *findRoutingTable(L3NetworkConfiguratorBase::Node *node) override;
    virtual bool linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule);
    virtual InterfaceInfo *findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node);

    // prefix template parsing
    virtual void parsePrefixTemplate(const char *prefixAttr, Ipv6Address& outPrefix, int& outPrefixLength, uint32_t& outSpecifiedGroups);
    virtual Ipv6Address generateUniquePrefix(const Ipv6Address& templatePrefix, int prefixLength, uint32_t specifiedGroups, const std::vector<Ipv6Address>& assignedPrefixes);
};

} // namespace inet

#endif
