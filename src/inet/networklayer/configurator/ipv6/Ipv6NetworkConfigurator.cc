//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.h"

#include <set>

#include "inet/common/XMLUtils.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"

namespace inet {

Define_Module(Ipv6NetworkConfigurator);

simsignal_t Ipv6NetworkConfigurator::networkConfigurationChangedSignal = cComponent::registerSignal("networkConfigurationChanged");

Ipv6NetworkConfigurator::InterfaceInfo::InterfaceInfo(Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface) :
    L3NetworkConfiguratorBase::InterfaceInfo(node, linkInfo, networkInterface)
{
}

void Ipv6NetworkConfigurator::initialize(int stage)
{
    L3NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        assignAddressesParameter = par("assignAddresses");
        assignAddressesToHostsParameter = par("assignAddressesToHosts");
        addStaticRoutesParameter = par("addStaticRoutes");
        addDefaultRoutesParameter = par("addDefaultRoutes");
        addDirectRoutesParameter = par("addDirectRoutes");
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        ensureConfigurationComputed(topology);
    }
    else if (stage == INITSTAGE_LAST)
        dumpConfiguration();
}

void Ipv6NetworkConfigurator::computeConfiguration()
{
    EV_INFO << "Computing static IPv6 network configuration (addresses and routes)" << endl;
    long initializeStartTime = clock();
    topology.clear();
    // extract topology into the Topology object, then fill in a LinkInfo[] vector
    TIME(extractTopology(topology));
    // read the configuration from XML; it will serve as input for prefix assignment
    TIME(readInterfaceConfiguration(topology));
    // assign prefixes and addresses to IPv6 interfaces
    if (assignAddressesParameter)
        TIME(assignAddresses(topology));
    // read and configure manual routes from the XML configuration
    readManualRouteConfiguration(topology);
    // read and configure manual multicast routes from the XML configuration
    readManualMulticastRouteConfiguration(topology);
    // calculate shortest paths, and add corresponding static routes
    if (addStaticRoutesParameter) {
        cXMLElementList autorouteElements = configuration->getChildrenByTagName("autoroute");
        if (autorouteElements.size() == 0) {
            cXMLElement defaultAutorouteElement("autoroute", "", nullptr);
            TIME(addStaticRoutes(topology, &defaultAutorouteElement));
        }
        else {
            for (auto& autorouteElement : autorouteElements)
                TIME(addStaticRoutes(topology, autorouteElement));
        }
    }
    emit(networkConfigurationChangedSignal, this);
    printElapsedTime("computeConfiguration", initializeStartTime);
}

void Ipv6NetworkConfigurator::ensureConfigurationComputed(Topology& topology)
{
    if (topology.getNumNodes() == 0)
        computeConfiguration();
}

void Ipv6NetworkConfigurator::dumpConfiguration()
{
    // print topology to module output
    if (par("dumpTopology"))
        TIME(dumpTopology(topology));
    // print links to module output
    if (par("dumpLinks"))
        TIME(dumpLinks(topology));
    // print unicast addresses to module output
    if (par("dumpAddresses"))
        TIME(dumpAddresses(topology));
    // print routes to module output
    if (par("dumpRoutes"))
        TIME(dumpRoutes(topology));
}

void Ipv6NetworkConfigurator::configureAllInterfaces()
{
    ensureConfigurationComputed(topology);
    EV_INFO << "Configuring all IPv6 interfaces" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (auto *ifInfo : node->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(ifInfo);
            if (interfaceInfo->configure)
                configureInterface(interfaceInfo);
        }
    }
}

void Ipv6NetworkConfigurator::configureInterface(NetworkInterface *networkInterface)
{
    ensureConfigurationComputed(topology);
    auto it = topology.interfaceInfos.find(networkInterface->getId());
    if (it != topology.interfaceInfos.end()) {
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(it->second);
        if (interfaceInfo->configure)
            configureInterface(interfaceInfo);
    }
}

void Ipv6NetworkConfigurator::configureAllRoutingTables()
{
    ensureConfigurationComputed(topology);
    EV_INFO << "Configuring all IPv6 routing tables" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable)
            configureRoutingTable(node);
    }
}

void Ipv6NetworkConfigurator::configureRoutingTable(Ipv6RoutingTable *routingTable)
{
    ensureConfigurationComputed(topology);
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable == routingTable) {
            configureRoutingTable(node);
            break;
        }
    }
}

void Ipv6NetworkConfigurator::configureInterface(InterfaceInfo *interfaceInfo)
{
    EV_DETAIL << "Configuring network interface " << interfaceInfo->getFullPath() << endl;
    NetworkInterface *networkInterface = interfaceInfo->networkInterface;
    auto ipv6Data = networkInterface->findProtocolDataForUpdate<Ipv6InterfaceData>();
    if (!ipv6Data)
        return;

    if (interfaceInfo->mtu != -1)
        networkInterface->setMtu(interfaceInfo->mtu);

    Node *node = static_cast<Node *>(interfaceInfo->node);
    bool isRouter = node->routingTable && node->routingTable->isForwardingEnabled();

    if (assignAddressesParameter && !interfaceInfo->globalAddress.isUnspecified()) {
        if (assignAddressesToHostsParameter || isRouter) {
            // assignAddressesToHosts: statically address every interface
            // !assignAddressesToHosts: address only routers; hosts use dynamic config (SLAAC/DHCPv6)
            if (!ipv6Data->hasAddress(interfaceInfo->globalAddress)) {
                ipv6Data->assignAddress(interfaceInfo->globalAddress, false, SIMTIME_ZERO, SIMTIME_ZERO);
                EV_DETAIL << "  assigned global address " << interfaceInfo->globalAddress << " to " << interfaceInfo->getFullPath() << endl;
            }
        }

        // For routers: set up AdvPrefix so they advertise the prefix
        if (isRouter) {
            Ipv6InterfaceData::AdvPrefix advPrefix;
            advPrefix.prefix = interfaceInfo->prefix;
            advPrefix.prefixLength = interfaceInfo->prefixLength;
            advPrefix.advOnLinkFlag = (interfaceInfo->advOnLinkFlag >= 0) ? (bool)interfaceInfo->advOnLinkFlag : true;
            advPrefix.advAutonomousFlag = (interfaceInfo->advAutonomousFlag >= 0) ? (bool)interfaceInfo->advAutonomousFlag : true;
            advPrefix.advValidLifetime = (interfaceInfo->advValidLifetime >= 0) ? interfaceInfo->advValidLifetime : 2592000; // 30 days
            advPrefix.advPreferredLifetime = (interfaceInfo->advPreferredLifetime >= 0) ? interfaceInfo->advPreferredLifetime : 604800; // 7 days
            advPrefix.advRtrAddr = false;

            // Check if this prefix is already advertised
            bool alreadyHasPrefix = false;
            for (int i = 0; i < ipv6Data->getNumAdvPrefixes(); i++) {
                if (ipv6Data->getAdvPrefix(i).prefix == interfaceInfo->prefix &&
                    ipv6Data->getAdvPrefix(i).prefixLength == interfaceInfo->prefixLength) {
                    alreadyHasPrefix = true;
                    break;
                }
            }
            if (!alreadyHasPrefix) {
                ipv6Data->addAdvPrefix(advPrefix);
                EV_DETAIL << "  added AdvPrefix " << interfaceInfo->prefix << "/" << interfaceInfo->prefixLength << " to " << interfaceInfo->getFullPath() << endl;
            }
        }
    }

    // Apply per-interface RA parameters from XML (independent of address assignment)
    if (isRouter) {
        if (interfaceInfo->advManagedFlag >= 0)
            ipv6Data->setAdvManagedFlag((bool)interfaceInfo->advManagedFlag);
        if (interfaceInfo->advOtherConfigFlag >= 0)
            ipv6Data->setAdvOtherConfigFlag((bool)interfaceInfo->advOtherConfigFlag);
        if (interfaceInfo->maxRtrAdvInterval >= 0)
            ipv6Data->setMaxRtrAdvInterval(SimTime(interfaceInfo->maxRtrAdvInterval));
        if (interfaceInfo->minRtrAdvInterval >= 0)
            ipv6Data->setMinRtrAdvInterval(SimTime(interfaceInfo->minRtrAdvInterval));
        if (interfaceInfo->advDefaultLifetime >= 0)
            ipv6Data->setAdvDefaultLifetime(SimTime(interfaceInfo->advDefaultLifetime));
    }

    // Apply per-interface DAD parameter from XML (applies to hosts and routers)
    if (interfaceInfo->dupAddrDetectTransmits >= 0)
        ipv6Data->setDupAddrDetectTransmits(interfaceInfo->dupAddrDetectTransmits);
}

void Ipv6NetworkConfigurator::configureRoutingTable(Node *node)
{
    auto routingTable = dynamic_cast<Ipv6RoutingTable *>(node->routingTable);
    if (!routingTable)
        return;

    auto nodePath = node->getModule()->getFullPath();
    EV_DETAIL << "Configuring routing table" << EV_FIELD(nodePath) << endl;

    // Add missing routes
    for (size_t i = 0; i < node->staticRoutes.size(); i++) {
        Ipv6Route *original = node->staticRoutes[i];
        if (contains(node->routingTableNetworkInterfaces, original->getInterface())) {
            // Check if route already exists
            bool found = false;
            for (int j = 0; j < routingTable->getNumRoutes(); j++) {
                Ipv6Route *existing = routingTable->getRoute(j);
                if (existing->getDestPrefix() == original->getDestPrefix() &&
                    existing->getPrefixLength() == original->getPrefixLength() &&
                    existing->getNextHop() == original->getNextHop() &&
                    existing->getInterface() == original->getInterface()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                Ipv6Route *route = new Ipv6Route(original->getDestPrefix(), original->getPrefixLength(), original->getSourceType());
                route->setNextHop(original->getNextHop());
                route->setInterface(original->getInterface());
                route->setMetric(original->getMetric());
                EV_DETAIL << "Adding route " << route->getDestPrefix() << "/" << route->getPrefixLength()
                          << " via " << route->getNextHop() << " dev " << route->getInterface()->getInterfaceName()
                          << " to " << nodePath << endl;
                routingTable->addStaticRoute(route->getDestPrefix(), route->getPrefixLength(),
                        route->getInterface()->getInterfaceId(), route->getNextHop(), route->getMetric());
                delete route;
            }
        }
    }

    // Add static multicast routes. The node owns the originals (computed once), so
    // install independent copies into the routing table; guard against re-adding if
    // this routing table was already configured (this method may run more than once).
    for (auto original : node->staticMulticastRoutes) {
        bool found = false;
        for (int j = 0; j < routingTable->getNumMulticastRoutes(); j++) {
            Ipv6MulticastRoute *existing = routingTable->getMulticastRoute(j);
            bool sameParent = (existing->getInInterface() == nullptr && original->getInInterface() == nullptr) ||
                    (existing->getInInterface() != nullptr && original->getInInterface() != nullptr &&
                     existing->getInInterface()->getInterface() == original->getInInterface()->getInterface());
            if (existing->getOrigin() == original->getOrigin() &&
                existing->getPrefixLength() == original->getPrefixLength() &&
                existing->getMulticastGroup() == original->getMulticastGroup() && sameParent) {
                found = true;
                break;
            }
        }
        if (!found) {
            EV_DETAIL << "Adding multicast route " << *original << " to " << nodePath << endl;
            routingTable->addMulticastRoute(new Ipv6MulticastRoute(*original));
        }
    }
}

Ipv6NetworkConfigurator::InterfaceInfo *Ipv6NetworkConfigurator::createInterfaceInfo(L3NetworkConfiguratorBase::Topology& topology, L3NetworkConfiguratorBase::Node *node, LinkInfo *linkInfo, NetworkInterface *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(static_cast<Ipv6NetworkConfigurator::Node *>(node), linkInfo, ie);
    node->interfaceInfos.push_back(interfaceInfo);
    topology.interfaceInfos[ie->getId()] = interfaceInfo;
    return interfaceInfo;
}

IRoutingTable *Ipv6NetworkConfigurator::findRoutingTable(L3NetworkConfiguratorBase::Node *node)
{
    return L3AddressResolver().findIpv6RoutingTableOf(node->module);
}

bool Ipv6NetworkConfigurator::linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule)
{
    for (auto& interfaceInfo : linkInfo->interfaceInfos) {
        cModule *hostModule = interfaceInfo->networkInterface->getInterfaceTable()->getHostModule();
        if (hostModule == exceptModule)
            continue;
        std::string hostFullPath = hostModule->getFullPath();
        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
        if (hostMatcher->matches(hostShortenedFullPath.c_str()) || hostMatcher->matches(hostFullPath.c_str()))
            return true;
    }
    return false;
}

Ipv6NetworkConfigurator::InterfaceInfo *Ipv6NetworkConfigurator::findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node)
{
    for (auto& interfaceInfo : linkInfo->interfaceInfos) {
        if (interfaceInfo->networkInterface->getInterfaceTable()->getHostModule() == node)
            return static_cast<InterfaceInfo *>(interfaceInfo);
    }
    return nullptr;
}

// Parse a prefix template like "2001:db8:x:x::/64"
// 'x' means unspecified 16-bit group
// outSpecifiedGroups is a bitmask: bit i (from MSB) = 1 means group i is specified
void Ipv6NetworkConfigurator::parsePrefixTemplate(const char *prefixAttr, Ipv6Address& outPrefix, int& outPrefixLength, uint32_t& outSpecifiedGroups)
{
    if (!prefixAttr || !*prefixAttr)
        throw cRuntimeError("Empty prefix attribute");

    std::string prefixStr(prefixAttr);
    outPrefixLength = 64; // default

    // Extract prefix length from "prefix/length" notation
    auto slashPos = prefixStr.find('/');
    if (slashPos != std::string::npos) {
        outPrefixLength = atoi(prefixStr.c_str() + slashPos + 1);
        prefixStr = prefixStr.substr(0, slashPos);
    }

    // Remove trailing :: if present
    bool hasTrailingDoubleColon = false;
    if (prefixStr.size() >= 2 && prefixStr.substr(prefixStr.size() - 2) == "::") {
        hasTrailingDoubleColon = true;
        prefixStr = prefixStr.substr(0, prefixStr.size() - 2);
    }

    // Split by ':' and parse each 16-bit group
    uint16_t groups16[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    outSpecifiedGroups = 0;

    std::vector<std::string> groups;
    std::stringstream ss(prefixStr);
    std::string group;
    while (std::getline(ss, group, ':'))
        groups.push_back(group);

    for (size_t i = 0; i < groups.size() && i < 8; i++) {
        if (groups[i] == "x" || groups[i] == "X") {
            groups16[i] = 0;
            // bit not set in outSpecifiedGroups => unspecified
        }
        else {
            groups16[i] = (uint16_t)strtoul(groups[i].c_str(), nullptr, 16);
            outSpecifiedGroups |= (1u << (7 - i)); // mark group i as specified
        }
    }

    // If there was a trailing ::, remaining groups are specified as 0
    if (hasTrailingDoubleColon) {
        for (size_t i = groups.size(); i < 8; i++)
            outSpecifiedGroups |= (1u << (7 - i));
    }

    // Pack 8 x 16-bit groups into 4 x 32-bit words for Ipv6Address constructor
    uint32_t d0 = ((uint32_t)groups16[0] << 16) | groups16[1];
    uint32_t d1 = ((uint32_t)groups16[2] << 16) | groups16[3];
    uint32_t d2 = ((uint32_t)groups16[4] << 16) | groups16[5];
    uint32_t d3 = ((uint32_t)groups16[6] << 16) | groups16[7];
    outPrefix = Ipv6Address(d0, d1, d2, d3);
}

Ipv6Address Ipv6NetworkConfigurator::generateUniquePrefix(const Ipv6Address& templatePrefix, int prefixLength, uint32_t specifiedGroups, const std::vector<Ipv6Address>& assignedPrefixes)
{
    // Determine which 16-bit groups within the prefix are unspecified
    // prefixLength tells us how many bits are in the network prefix (typically 64)
    int prefixGroups = (prefixLength + 15) / 16; // number of 16-bit groups in the prefix

    // Find unspecified groups within the prefix portion
    std::vector<int> freeGroupIndices;
    for (int i = 0; i < prefixGroups; i++) {
        if (!(specifiedGroups & (1u << (7 - i))))
            freeGroupIndices.push_back(i);
    }

    if (freeGroupIndices.empty())
        return templatePrefix; // fully specified, return as-is

    // Extract 16-bit groups from template's 4 x 32-bit words
    const uint32_t *raw = templatePrefix.words();
    uint16_t groups16[8];
    for (int i = 0; i < 4; i++) {
        groups16[i * 2] = (raw[i] >> 16) & 0xFFFF;
        groups16[i * 2 + 1] = raw[i] & 0xFFFF;
    }

    // Try values starting from 1
    for (uint32_t counter = 1; counter < 0xFFFF; counter++) {
        // Distribute counter across free groups (simple: put it all in the last free group)
        // For multiple free groups, use a multi-digit counter
        uint16_t testGroups[8];
        memcpy(testGroups, groups16, sizeof(testGroups));

        uint32_t remaining = counter;
        for (int i = (int)freeGroupIndices.size() - 1; i >= 0; i--) {
            testGroups[freeGroupIndices[i]] = remaining & 0xFFFF;
            remaining >>= 16;
        }

        // Pack back into 4 x 32-bit for Ipv6Address constructor
        uint32_t d0 = ((uint32_t)testGroups[0] << 16) | testGroups[1];
        uint32_t d1 = ((uint32_t)testGroups[2] << 16) | testGroups[3];
        uint32_t d2 = ((uint32_t)testGroups[4] << 16) | testGroups[5];
        uint32_t d3 = ((uint32_t)testGroups[6] << 16) | testGroups[7];
        Ipv6Address candidate(d0, d1, d2, d3);

        // Check that this prefix is not already assigned
        bool unique = true;
        for (auto& assigned : assignedPrefixes) {
            if (candidate.getPrefix(prefixLength) == assigned.getPrefix(prefixLength)) {
                unique = false;
                break;
            }
        }
        if (unique)
            return candidate;
    }

    throw cRuntimeError("Failed to generate unique IPv6 prefix from template");
}

void Ipv6NetworkConfigurator::readInterfaceConfiguration(Topology& topology)
{
    using namespace xmlutils;

    std::set<InterfaceInfo *> interfacesSeen;
    cXMLElementList interfaceElements = configuration->getChildrenByTagName("interface");
    EV_INFO << "Reading IPv6 interface configuration of " << interfaceElements.size() << " XML elements" << endl;

    for (auto& interfaceElement : interfaceElements) {
        const char *hostAttr = interfaceElement->getAttribute("hosts");
        const char *interfaceAttr = interfaceElement->getAttribute("names");
        const char *towardsAttr = interfaceElement->getAttribute("towards");
        const char *amongAttr = interfaceElement->getAttribute("among");
        const char *prefixAttr = interfaceElement->getAttribute("prefix");
        const char *addressAttr = interfaceElement->getAttribute("address");
        const char *mtuAttr = interfaceElement->getAttribute("mtu");
        const char *metricAttr = interfaceElement->getAttribute("metric");
        bool addStaticRouteAttr = getAttributeBoolValue(interfaceElement, "add-static-route", true);
        bool addDefaultRouteAttr = getAttributeBoolValue(interfaceElement, "add-default-route", true);

        // RA/NDP optional attributes
        const char *advValidLifetimeAttr = interfaceElement->getAttribute("advValidLifetime");
        const char *advPreferredLifetimeAttr = interfaceElement->getAttribute("advPreferredLifetime");
        const char *advOnLinkFlagAttr = interfaceElement->getAttribute("advOnLinkFlag");
        const char *advAutonomousFlagAttr = interfaceElement->getAttribute("advAutonomousFlag");
        const char *advManagedFlagAttr = interfaceElement->getAttribute("advManagedFlag");
        const char *advOtherConfigFlagAttr = interfaceElement->getAttribute("advOtherConfigFlag");
        const char *maxRtrAdvIntervalAttr = interfaceElement->getAttribute("maxRtrAdvInterval");
        const char *minRtrAdvIntervalAttr = interfaceElement->getAttribute("minRtrAdvInterval");
        const char *advDefaultLifetimeAttr = interfaceElement->getAttribute("advDefaultLifetime");
        const char *dupAddrDetectTransmitsAttr = interfaceElement->getAttribute("dupAddrDetectTransmits");

        if (amongAttr && *amongAttr) {
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", interfaceElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try {
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);
            Matcher towardsMatcher(towardsAttr);

            bool havePrefixConstraint = !opp_isempty(prefixAttr);
            Ipv6Address prefixTemplate;
            int prefixLength = 64;
            uint32_t specifiedGroups = 0;
            if (havePrefixConstraint)
                parsePrefixTemplate(prefixAttr, prefixTemplate, prefixLength, specifiedGroups);

            // An explicit full address (e.g. "2001:db8:1:1::1") overrides the EUI-64 interface id.
            bool haveAddressConstraint = !opp_isempty(addressAttr);
            Ipv6Address explicitAddress;
            if (haveAddressConstraint)
                explicitAddress = Ipv6Address(addressAttr);

            for (auto& linkInfo : topology.linkInfos) {
                for (size_t j = 0; j < linkInfo->interfaceInfos.size(); j++) {
                    InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(linkInfo->interfaceInfos[j]);
                    if (interfacesSeen.count(interfaceInfo) == 0) {
                        cModule *hostModule = interfaceInfo->networkInterface->getInterfaceTable()->getHostModule();
                        std::string hostFullPath = hostModule->getFullPath();
                        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                        if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                            (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceInfo->networkInterface->getInterfaceName())) &&
                            (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(linkInfo, &towardsMatcher, hostModule)))
                        {
                            EV_DEBUG << "Processing interface configuration for " << interfaceInfo->getFullPath() << " using " << interfaceElement->str() << endl;

                            interfaceInfo->configure = true;
                            if (havePrefixConstraint) {
                                interfaceInfo->prefix = prefixTemplate;
                                interfaceInfo->prefixLength = prefixLength;
                                interfaceInfo->specifiedGroups = specifiedGroups;
                            }
                            if (haveAddressConstraint)
                                interfaceInfo->explicitAddress = explicitAddress;

                            interfaceInfo->addStaticRoute = addStaticRouteAttr;
                            interfaceInfo->addDefaultRoute = addDefaultRouteAttr;

                            if (!opp_isempty(mtuAttr))
                                interfaceInfo->mtu = atoi(mtuAttr);
                            if (!opp_isempty(metricAttr))
                                interfaceInfo->metric = atoi(metricAttr);

                            // Store RA/NDP parameters
                            if (!opp_isempty(advValidLifetimeAttr))
                                interfaceInfo->advValidLifetime = atoi(advValidLifetimeAttr);
                            if (!opp_isempty(advPreferredLifetimeAttr))
                                interfaceInfo->advPreferredLifetime = atoi(advPreferredLifetimeAttr);
                            if (!opp_isempty(advOnLinkFlagAttr))
                                interfaceInfo->advOnLinkFlag = getAttributeBoolValue(interfaceElement, "advOnLinkFlag", true) ? 1 : 0;
                            if (!opp_isempty(advAutonomousFlagAttr))
                                interfaceInfo->advAutonomousFlag = getAttributeBoolValue(interfaceElement, "advAutonomousFlag", true) ? 1 : 0;
                            if (!opp_isempty(advManagedFlagAttr))
                                interfaceInfo->advManagedFlag = getAttributeBoolValue(interfaceElement, "advManagedFlag", false) ? 1 : 0;
                            if (!opp_isempty(advOtherConfigFlagAttr))
                                interfaceInfo->advOtherConfigFlag = getAttributeBoolValue(interfaceElement, "advOtherConfigFlag", false) ? 1 : 0;
                            if (!opp_isempty(maxRtrAdvIntervalAttr))
                                interfaceInfo->maxRtrAdvInterval = atof(maxRtrAdvIntervalAttr);
                            if (!opp_isempty(minRtrAdvIntervalAttr))
                                interfaceInfo->minRtrAdvInterval = atof(minRtrAdvIntervalAttr);
                            if (!opp_isempty(advDefaultLifetimeAttr))
                                interfaceInfo->advDefaultLifetime = atoi(advDefaultLifetimeAttr);
                            if (!opp_isempty(dupAddrDetectTransmitsAttr))
                                interfaceInfo->dupAddrDetectTransmits = atoi(dupAddrDetectTransmitsAttr);

                            interfacesSeen.insert(interfaceInfo);
                        }
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <interface> element at %s: %s", interfaceElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv6NetworkConfigurator::readManualRouteConfiguration(Topology& topology)
{
    using namespace xmlutils;

    cXMLElementList routeElements = configuration->getChildrenByTagName("route");
    EV_INFO << "Reading manual route configuration of " << routeElements.size() << " XML elements" << endl;

    for (auto& routeElement : routeElements) {
        const char *hostAttr = routeElement->getAttribute("hosts");
        const char *destinationAttr = routeElement->getAttribute("destination");
        const char *prefixLengthAttr = routeElement->getAttribute("prefixLength");
        const char *gatewayAttr = routeElement->getAttribute("gateway");
        const char *interfaceAttr = routeElement->getAttribute("interface");
        const char *metricAttr = routeElement->getAttribute("metric");

        if (opp_isempty(destinationAttr))
            throw cRuntimeError("Missing 'destination' attribute in <route> at %s", routeElement->getSourceLocation());

        try {
            Matcher hostMatcher(hostAttr);

            // The prefix length may be given either as a separate 'prefixLength'
            // attribute or appended to 'destination' in "address/prefixlen" notation;
            // the two are equivalent, but specifying both is an error.
            std::string destinationStr = destinationAttr;
            int prefixLength;
            auto slashPos = destinationStr.find('/');
            if (slashPos != std::string::npos) {
                if (!opp_isempty(prefixLengthAttr))
                    throw cRuntimeError("Give the prefix length either in 'destination' as \"address/prefixlen\" or in a separate 'prefixLength' attribute, not both, in <route> at %s", routeElement->getSourceLocation());
                prefixLength = atoi(destinationStr.substr(slashPos + 1).c_str());
                destinationStr = destinationStr.substr(0, slashPos);
            }
            else
                prefixLength = prefixLengthAttr ? atoi(prefixLengthAttr) : 64;

            int metric = metricAttr ? atoi(metricAttr) : 0;

            for (int i = 0; i < topology.getNumNodes(); i++) {
                Node *node = (Node *)topology.getNode(i);
                if (!node->routingTable)
                    continue;

                std::string hostFullPath = node->module->getFullPath();
                std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                if (!hostMatcher.matchesAny() && !hostMatcher.matches(hostShortenedFullPath.c_str()) && !hostMatcher.matches(hostFullPath.c_str()))
                    continue;

                // Resolve destination
                Ipv6Address destination;
                if (destinationStr == "*" || destinationStr == "::")
                    destination = Ipv6Address::UNSPECIFIED_ADDRESS;
                else {
                    L3Address addr = L3AddressResolver().resolve(destinationStr.c_str(), L3AddressResolver::ADDR_IPv6);
                    destination = addr.toIpv6();
                }

                // Resolve gateway
                Ipv6Address gateway;
                if (!opp_isempty(gatewayAttr) && strcmp(gatewayAttr, "*") != 0) {
                    L3Address addr = L3AddressResolver().resolve(gatewayAttr, L3AddressResolver::ADDR_IPv6);
                    gateway = addr.toIpv6();
                }

                // Resolve interface
                NetworkInterface *outInterface = nullptr;
                if (!opp_isempty(interfaceAttr)) {
                    IInterfaceTable *ift = node->interfaceTable;
                    outInterface = ift->findInterfaceByName(interfaceAttr);
                    if (!outInterface)
                        throw cRuntimeError("Interface '%s' not found in node '%s'", interfaceAttr, hostFullPath.c_str());
                }

                if (!outInterface && gateway.isUnspecified())
                    throw cRuntimeError("Either 'gateway' or 'interface' must be specified in <route> at %s", routeElement->getSourceLocation());

                // Create route
                Ipv6Route *route = new Ipv6Route(destination, prefixLength, IRoute::MANUAL);
                route->setNextHop(gateway);
                if (outInterface)
                    route->setInterface(outInterface);
                route->setMetric(metric);
                node->staticRoutes.push_back(route);
                if (outInterface)
                    node->routingTableNetworkInterfaces.push_back(outInterface);
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv6NetworkConfigurator::readManualMulticastRouteConfiguration(Topology& topology)
{
    cXMLElementList routeElements = configuration->getChildrenByTagName("multicast-route");
    EV_INFO << "Reading IPv6 multicast route configuration of " << routeElements.size() << " XML elements" << endl;

    for (auto& routeElement : routeElements) {
        const char *hostAttr = routeElement->getAttribute("hosts");
        const char *sourceAttr = routeElement->getAttribute("source"); // source address (L3AddressResolver syntax), default: all sources
        const char *prefixLengthAttr = routeElement->getAttribute("prefixLength"); // length of source prefix to match
        const char *groupsAttr = routeElement->getAttribute("groups"); // multicast group addresses, default: all groups
        const char *parentAttr = routeElement->getAttribute("parent"); // name of expected (RPF) input interface
        const char *childrenAttr = routeElement->getAttribute("children"); // names of output interfaces
        const char *metricAttr = routeElement->getAttribute("metric");

        try {
            // parse the source prefix (origin); empty or "*" means "any source"
            Ipv6Address source = Ipv6Address::UNSPECIFIED_ADDRESS;
            int prefixLength = 0;
            if (!opp_isempty(sourceAttr) && strcmp(sourceAttr, "*") != 0) {
                source = L3AddressResolver().resolve(sourceAttr, L3AddressResolver::ADDR_IPv6).toIpv6();
                prefixLength = !opp_isempty(prefixLengthAttr) ? atoi(prefixLengthAttr) : 128;
            }

            // parse the group addresses; empty means "any group"
            std::vector<Ipv6Address> groups;
            if (opp_isempty(groupsAttr))
                groups.push_back(Ipv6Address::UNSPECIFIED_ADDRESS);
            else {
                cStringTokenizer tokenizer(groupsAttr);
                while (tokenizer.hasMoreTokens()) {
                    Ipv6Address group = Ipv6Address(tokenizer.nextToken());
                    if (!group.isMulticast())
                        throw cRuntimeError("Address '%s' in 'groups' attribute is not a multicast address", group.str().c_str());
                    groups.push_back(group);
                }
            }

            int metric = !opp_isempty(metricAttr) ? atoi(metricAttr) : 0;

            // find matching host(s) and add the route
            Matcher hostMatcher(hostAttr);
            InterfaceMatcher childrenMatcher(childrenAttr);
            for (int i = 0; i < topology.getNumNodes(); i++) {
                Node *node = (Node *)topology.getNode(i);
                // Install the route on any node with a routing table that matches the
                // host pattern. Unlike Ipv4NetworkConfigurator we do not also require
                // isMulticastForwardingEnabled() here: that flag is not reliably set at
                // configuration time, and a multicast route on a non-forwarding node is
                // harmless (Ipv6::routeMulticastPacket gates forwarding on isRouter()).
                if (!node->routingTable)
                    continue;

                std::string hostFullPath = node->module->getFullPath();
                std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                if (!hostMatcher.matchesAny() && !hostMatcher.matches(hostShortenedFullPath.c_str()) && !hostMatcher.matches(hostFullPath.c_str()))
                    continue;

                // resolve the parent (RPF) interface
                NetworkInterface *parent = nullptr;
                if (!opp_isempty(parentAttr)) {
                    parent = node->interfaceTable->findInterfaceByName(parentAttr);
                    if (!parent)
                        throw cRuntimeError("Parent interface '%s' not found in node '%s'", parentAttr, hostFullPath.c_str());
                    if (!parent->isMulticast())
                        throw cRuntimeError("Parent interface '%s' in node '%s' is not multicast", parentAttr, hostFullPath.c_str());
                }

                // collect the child (output) interfaces
                std::vector<NetworkInterface *> children;
                for (auto& element : node->interfaceInfos) {
                    InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
                    NetworkInterface *ie = interfaceInfo->networkInterface;
                    if (ie != parent && ie->isMulticast() && childrenMatcher.matches(interfaceInfo))
                        children.push_back(ie);
                }

                for (auto& group : groups) {
                    Ipv6MulticastRoute *route = new Ipv6MulticastRoute();
                    route->setSourceType(IMulticastRoute::MANUAL);
                    route->setSource(this);
                    route->setOrigin(source);
                    route->setPrefixLength(prefixLength);
                    route->setMulticastGroup(group);
                    route->setMetric(metric);
                    if (parent) {
                        route->setInInterface(new Ipv6MulticastRoute::InInterface(parent));
                        node->routingTableNetworkInterfaces.push_back(parent);
                    }
                    for (auto& child : children) {
                        route->addOutInterface(new Ipv6MulticastRoute::OutInterface(child, false /*TODO isLeaf*/));
                        node->routingTableNetworkInterfaces.push_back(child);
                    }
                    EV_INFO << "Adding manual IPv6 multicast route " << *route << " to " << hostFullPath << endl;
                    node->staticMulticastRoutes.push_back(route);
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <multicast-route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv6NetworkConfigurator::assignAddresses(Topology& topology)
{
    EV_INFO << "Assigning IPv6 prefixes and addresses" << endl;

    std::vector<Ipv6Address> assignedPrefixes;

    // Assign a unique prefix to each link, then derive global addresses for each interface on that link
    for (auto& linkInfo : topology.linkInfos) {
        if (linkInfo->interfaceInfos.empty())
            continue;

        // Find a prefix source for this link: the first configured interface that carries either a
        // prefix template or an explicit @address (an explicit address seeds the link prefix too).
        InterfaceInfo *templateInterface = nullptr;
        for (auto& ifInfo : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(ifInfo);
            if (interfaceInfo->configure && (!interfaceInfo->prefix.isUnspecified() || !interfaceInfo->explicitAddress.isUnspecified())) {
                templateInterface = interfaceInfo;
                break;
            }
        }
        if (!templateInterface)
            continue;

        int prefixLength = templateInterface->prefixLength;

        // Derive the link prefix from the prefix template if present, otherwise from the explicit address.
        Ipv6Address linkPrefix;
        if (!templateInterface->prefix.isUnspecified())
            linkPrefix = generateUniquePrefix(templateInterface->prefix, prefixLength, templateInterface->specifiedGroups, assignedPrefixes);
        else
            linkPrefix = templateInterface->explicitAddress.getPrefix(prefixLength);
        assignedPrefixes.push_back(linkPrefix);

        EV_DEBUG << "Link gets prefix " << linkPrefix << "/" << prefixLength << endl;

        // Assign the same prefix to all interfaces on this link
        for (auto& ifInfo : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(ifInfo);
            if (!interfaceInfo->configure)
                continue;

            interfaceInfo->prefix = linkPrefix;
            interfaceInfo->prefixLength = prefixLength;

            if (!interfaceInfo->explicitAddress.isUnspecified()) {
                // An explicit @address overrides the EUI-64 interface id; it must lie on the link prefix.
                if (interfaceInfo->explicitAddress.getPrefix(prefixLength) != linkPrefix)
                    throw cRuntimeError("Explicit address %s on %s is not within the link prefix %s/%d",
                            interfaceInfo->explicitAddress.str().c_str(), interfaceInfo->getFullPath().c_str(),
                            linkPrefix.str().c_str(), prefixLength);
                interfaceInfo->globalAddress = interfaceInfo->explicitAddress;
            }
            else {
                // Derive global address: prefix + interface token (IID)
                NetworkInterface *networkInterface = interfaceInfo->networkInterface;
                InterfaceToken token = networkInterface->getInterfaceToken();

                // Build global address from prefix and IID
                const uint32_t *prefixRaw = linkPrefix.words();
                uint32_t addr[4];
                addr[0] = prefixRaw[0];
                addr[1] = prefixRaw[1];
                // The lower 64 bits come from the interface token
                // InterfaceToken: normal() = upper 32 bits of IID, low() = lower 32 bits
                if (token.length() > 0) {
                    addr[2] = token.normal();
                    addr[3] = token.low();
                }
                else {
                    // Fallback: use interface ID to generate a unique IID
                    addr[2] = 0;
                    addr[3] = networkInterface->getInterfaceId();
                }
                interfaceInfo->globalAddress = Ipv6Address(addr[0], addr[1], addr[2], addr[3]);
            }

            EV_DEBUG << "  Interface " << interfaceInfo->getFullPath()
                     << " gets address " << interfaceInfo->globalAddress << "/" << prefixLength << endl;
        }
    }
}

void Ipv6NetworkConfigurator::addStaticRoutes(Topology& topology, cXMLElement *autorouteElement)
{
    EV_INFO << "Adding IPv6 static routes" << endl;

    // set node weights
    const char *metric = autorouteElement->getAttribute("metric");
    if (metric == nullptr)
        metric = "hopCount";
    cXMLElement defaultNodeElement("node", "", nullptr);
    cXMLElementList nodeElements = autorouteElement->getChildrenByTagName("node");
    for (int i = 0; i < topology.getNumNodes(); i++) {
        cXMLElement *selectedNodeElement = &defaultNodeElement;
        Node *node = (Node *)topology.getNode(i);
        for (auto& nodeElement : nodeElements) {
            const char *hosts = nodeElement->getAttribute("hosts");
            if (hosts == nullptr)
                hosts = "**";
            Matcher nodeHostsMatcher(hosts);
            std::string hostFullPath = node->module->getFullPath();
            std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
            if (nodeHostsMatcher.matchesAny() || nodeHostsMatcher.matches(hostShortenedFullPath.c_str()) || nodeHostsMatcher.matches(hostFullPath.c_str())) {
                selectedNodeElement = nodeElement;
                break;
            }
        }
        double weight = computeNodeWeight(node, metric, selectedNodeElement);
        EV_DEBUG << "Setting node weight, node = " << node->module->getFullPath() << ", weight = " << weight << endl;
        node->setWeight(weight);
    }

    // set link weights
    cXMLElement defaultLinkElement("link", "", nullptr);
    cXMLElementList linkElements = autorouteElement->getChildrenByTagName("link");
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumInLinks(); j++) {
            cXMLElement *selectedLinkElement = &defaultLinkElement;
            Link *link = (Link *)node->getLinkIn(j);
            for (auto& linkElement : linkElements) {
                const char *interfaces = linkElement->getAttribute("interfaces");
                if (interfaces == nullptr)
                    interfaces = "**";
                Matcher linkInterfaceMatcher(interfaces);
                std::string sourceFullPath = link->sourceInterfaceInfo->getFullPath();
                std::string sourceShortenedFullPath = sourceFullPath.substr(sourceFullPath.find('.') + 1);
                std::string destinationFullPath = link->destinationInterfaceInfo->getFullPath();
                std::string destinationShortenedFullPath = destinationFullPath.substr(destinationFullPath.find('.') + 1);
                if (linkInterfaceMatcher.matchesAny() ||
                    linkInterfaceMatcher.matches(sourceFullPath.c_str()) || linkInterfaceMatcher.matches(sourceShortenedFullPath.c_str()) ||
                    linkInterfaceMatcher.matches(destinationFullPath.c_str()) || linkInterfaceMatcher.matches(destinationShortenedFullPath.c_str()))
                {
                    selectedLinkElement = linkElement;
                    break;
                }
            }
            double weight = computeLinkWeight(link, metric, selectedLinkElement);
            EV_DEBUG << "Setting link weight, link = " << link << ", weight = " << weight << endl;
            link->setWeight(weight);
        }
    }

    // add static routes for all routing tables
    const char *sourceHosts = autorouteElement->getAttribute("sourceHosts");
    if (sourceHosts == nullptr)
        sourceHosts = "**";
    const char *destinationInterfaces = autorouteElement->getAttribute("destinationInterfaces");
    if (destinationInterfaces == nullptr)
        destinationInterfaces = "**";
    Matcher sourceHostsMatcher(sourceHosts);
    Matcher destinationInterfacesMatcher(destinationInterfaces);

    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *sourceNode = (Node *)topology.getNode(i);
        std::string hostFullPath = sourceNode->module->getFullPath();
        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
        if (!sourceHostsMatcher.matchesAny() && !sourceHostsMatcher.matches(hostShortenedFullPath.c_str()) && !sourceHostsMatcher.matches(hostFullPath.c_str()))
            continue;
        if (isBridgeNode(sourceNode))
            continue;
        if (!sourceNode->routingTable)
            continue;

        // calculate shortest paths from everywhere to sourceNode
        topology.calculateWeightedSingleShortestPathsTo(sourceNode);

        // check if adding a default route would be ok (optimization for single-interface hosts)
        if (addDefaultRoutesParameter && sourceNode->interfaceInfos.size() == 1 && sourceNode->interfaceInfos[0]->linkInfo->gatewayInterfaceInfo && sourceNode->interfaceInfos[0]->addDefaultRoute) {
            InterfaceInfo *sourceInterfaceInfo = static_cast<InterfaceInfo *>(sourceNode->interfaceInfos[0]);
            NetworkInterface *sourceNetworkInterface = sourceInterfaceInfo->networkInterface;
            InterfaceInfo *gatewayInterfaceInfo = static_cast<InterfaceInfo *>(sourceInterfaceInfo->linkInfo->gatewayInterfaceInfo);

            if (addDirectRoutesParameter) {
                // add an on-link route for the local network prefix
                Ipv6Route *route = new Ipv6Route(sourceInterfaceInfo->prefix, sourceInterfaceInfo->prefixLength, IRoute::MANUAL);
                route->setNextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
                route->setInterface(sourceNetworkInterface);
                route->setMetric(0);
                EV_DEBUG << "Adding direct route " << sourceInterfaceInfo->prefix << "/" << sourceInterfaceInfo->prefixLength
                         << " to " << sourceNode->module->getFullPath() << endl;
                sourceNode->staticRoutes.push_back(route);
                sourceNode->routingTableNetworkInterfaces.push_back(route->getInterface());
            }

            // add default route towards the gateway (using gateway's link-local address as next hop)
            Ipv6Address gatewayLinkLocal;
            auto gwIpv6Data = gatewayInterfaceInfo->networkInterface->findProtocolData<Ipv6InterfaceData>();
            if (gwIpv6Data)
                gatewayLinkLocal = gwIpv6Data->getLinkLocalAddress();
            if (gatewayLinkLocal.isUnspecified())
                gatewayLinkLocal = gatewayInterfaceInfo->globalAddress; // fallback to global

            Ipv6Route *route = new Ipv6Route(Ipv6Address::UNSPECIFIED_ADDRESS, 0, IRoute::MANUAL);
            route->setNextHop(gatewayLinkLocal);
            route->setInterface(sourceNetworkInterface);
            route->setMetric(0);
            EV_DEBUG << "Adding default route via " << gatewayLinkLocal << " to " << sourceNode->module->getFullPath() << endl;
            sourceNode->staticRoutes.push_back(route);
            sourceNode->routingTableNetworkInterfaces.push_back(route->getInterface());

            EV_DEBUG << "Adding default routes to " << sourceNode->getModule()->getFullPath() << ", node has only one (non-loopback) interface" << endl;
        }
        else {
            // add a route to all destinations in the network
            for (int j = 0; j < topology.getNumNodes(); j++) {
                Node *destinationNode = (Node *)topology.getNode(j);
                if (sourceNode == destinationNode)
                    continue;
                if (destinationNode->getNumPaths() == 0)
                    continue;
                if (isBridgeNode(destinationNode))
                    continue;
                if (std::isinf(destinationNode->getDistanceToTarget()))
                    continue;

                // determine next hop interface
                Node *node = destinationNode;
                Link *link = nullptr;
                InterfaceInfo *nextHopInterfaceInfo = nullptr;
                while (node != sourceNode) {
                    link = (Link *)node->getPath(0);
                    if (node != sourceNode && !isBridgeNode(node) && link->sourceInterfaceInfo)
                        nextHopInterfaceInfo = static_cast<InterfaceInfo *>(link->sourceInterfaceInfo);
                    node = (Node *)node->getPath(0)->getLinkOutRemoteNode();
                }

                if (nextHopInterfaceInfo && link->destinationInterfaceInfo && link->destinationInterfaceInfo->addStaticRoute) {
                    NetworkInterface *sourceNetworkInterface = link->destinationInterfaceInfo->networkInterface;

                    // add routes for all destination interfaces
                    for (size_t k = 0; k < destinationNode->interfaceInfos.size(); k++) {
                        InterfaceInfo *destinationInterfaceInfo = static_cast<InterfaceInfo *>(destinationNode->interfaceInfos[k]);
                        std::string destinationFullPath = destinationInterfaceInfo->networkInterface->getInterfaceFullPath();
                        std::string destinationShortenedFullPath = destinationFullPath.substr(destinationFullPath.find('.') + 1);
                        if (!destinationInterfacesMatcher.matchesAny() &&
                            !destinationInterfacesMatcher.matches(destinationFullPath.c_str()) &&
                            !destinationInterfacesMatcher.matches(destinationShortenedFullPath.c_str()))
                            continue;

                        NetworkInterface *destinationNetworkInterface = destinationInterfaceInfo->networkInterface;
                        if (destinationNetworkInterface->isLoopback())
                            continue;
                        if (destinationInterfaceInfo->globalAddress.isUnspecified())
                            continue;

                        // Use the on-link prefix as the route destination
                        Ipv6Address destPrefix = destinationInterfaceInfo->prefix;
                        int destPrefixLength = destinationInterfaceInfo->prefixLength;

                        // If THIS destination interface sits on a link the source node is
                        // itself attached to, install a DIRECT (on-link) route -- unspecified
                        // next hop, output on the source's interface on that link -- so the
                        // source resolves the destination directly via Neighbour Discovery.
                        // A "via <host>" route would be wrong on a link shared by several
                        // hosts: each host's prefix route would point at whichever host was
                        // added first, so traffic to the other hosts would be misdelivered
                        // to it. A direct route is also correct and de-duplicates: all hosts
                        // on one link collapse to a single on-link prefix route.
                        NetworkInterface *directOutInterface = nullptr;
                        for (auto& ifInfoBase : destinationInterfaceInfo->linkInfo->interfaceInfos) {
                            InterfaceInfo *onLinkIf = static_cast<InterfaceInfo *>(ifInfoBase);
                            if (static_cast<Node *>(onLinkIf->node) == sourceNode) {
                                directOutInterface = onLinkIf->networkInterface;
                                break;
                            }
                        }

                        Ipv6Address nextHop;
                        NetworkInterface *outInterface;
                        if (directOutInterface != nullptr)
                            outInterface = directOutInterface; // on-link: resolve the destination directly
                        else {
                            outInterface = sourceNetworkInterface;
                            auto nhIpv6Data = nextHopInterfaceInfo->networkInterface->findProtocolData<Ipv6InterfaceData>();
                            if (nhIpv6Data)
                                nextHop = nhIpv6Data->getLinkLocalAddress();
                            if (nextHop.isUnspecified())
                                nextHop = nextHopInterfaceInfo->globalAddress; // fallback
                        }

                        // Skip direct (on-link) routes when direct routes are disabled
                        if (!addDirectRoutesParameter && nextHop.isUnspecified())
                            continue;

                        // Check for duplicate routes
                        bool duplicate = false;
                        for (auto *existingRoute : sourceNode->staticRoutes) {
                            if (existingRoute->getDestPrefix() == destPrefix &&
                                existingRoute->getPrefixLength() == destPrefixLength &&
                                existingRoute->getNextHop() == nextHop &&
                                existingRoute->getInterface() == outInterface) {
                                duplicate = true;
                                break;
                            }
                        }
                        if (duplicate)
                            continue;

                        Ipv6Route *route = new Ipv6Route(destPrefix, destPrefixLength, IRoute::MANUAL);
                        route->setNextHop(nextHop);
                        route->setInterface(outInterface);
                        route->setMetric(0);
                        sourceNode->staticRoutes.push_back(route);
                        sourceNode->routingTableNetworkInterfaces.push_back(route->getInterface());
                        EV_DETAIL << "Adding route " << outInterface->getInterfaceFullPath() << " -> "
                                  << destinationNetworkInterface->getInterfaceFullPath() << " as "
                                  << destPrefix << "/" << destPrefixLength;
                        if (nextHop.isUnspecified())
                            EV_DETAIL << " (on-link)" << endl;
                        else
                            EV_DETAIL << " via " << nextHop << endl;
                    }
                }
            }
        }

        // The loops above only add routes toward other CONFIGURED interfaces. A router
        // interface on a link with no other configured member -- e.g. an access router
        // serving a wireless link whose hosts autoconfigure (SLAAC) only at runtime --
        // would then have no route to its own on-link prefix and could not deliver to
        // those hosts. Fill such gaps with on-link (direct) routes. Prefixes that already
        // have a route (every multi-member, e.g. wired, link) are left untouched.
        if (addDirectRoutesParameter && sourceNode->routingTable) {
            for (size_t k = 0; k < sourceNode->interfaceInfos.size(); k++) {
                InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(sourceNode->interfaceInfos[k]);
                NetworkInterface *networkInterface = interfaceInfo->networkInterface;
                if (networkInterface->isLoopback())
                    continue;
                if (interfaceInfo->prefix.isUnspecified() || interfaceInfo->globalAddress.isUnspecified())
                    continue;
                bool haveRoute = false;
                for (auto *existingRoute : sourceNode->staticRoutes)
                    if (existingRoute->getDestPrefix() == interfaceInfo->prefix &&
                        existingRoute->getPrefixLength() == interfaceInfo->prefixLength)
                    {
                        haveRoute = true;
                        break;
                    }
                if (haveRoute)
                    continue;
                Ipv6Route *route = new Ipv6Route(interfaceInfo->prefix, interfaceInfo->prefixLength, IRoute::MANUAL);
                route->setNextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
                route->setInterface(networkInterface);
                route->setMetric(0);
                EV_DEBUG << "Adding on-link route " << interfaceInfo->prefix << "/" << interfaceInfo->prefixLength
                         << " dev " << networkInterface->getInterfaceName() << " to " << sourceNode->module->getFullPath()
                         << " (directly-connected prefix with no other configured member)" << endl;
                sourceNode->staticRoutes.push_back(route);
                sourceNode->routingTableNetworkInterfaces.push_back(route->getInterface());
            }
        }
    }
}

void Ipv6NetworkConfigurator::dumpLinks(Topology& topology)
{
    EV_INFO << "--- IPv6 Links ---" << endl;
    for (auto& linkInfo : topology.linkInfos) {
        EV_INFO << "Link (network id " << linkInfo->networkId << "):" << endl;
        for (auto& ifInfo : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(ifInfo);
            EV_INFO << "  " << interfaceInfo->getFullPath();
            if (!interfaceInfo->prefix.isUnspecified())
                EV_INFO << " prefix=" << interfaceInfo->prefix << "/" << interfaceInfo->prefixLength;
            EV_INFO << endl;
        }
    }
}

void Ipv6NetworkConfigurator::dumpAddresses(Topology& topology)
{
    EV_INFO << "--- IPv6 Addresses ---" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        EV_INFO << node->module->getFullPath() << ":" << endl;
        for (auto *ifInfo : node->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(ifInfo);
            EV_INFO << "  " << interfaceInfo->networkInterface->getInterfaceName();
            if (!interfaceInfo->globalAddress.isUnspecified())
                EV_INFO << " addr=" << interfaceInfo->globalAddress << "/" << interfaceInfo->prefixLength;
            if (!interfaceInfo->prefix.isUnspecified())
                EV_INFO << " prefix=" << interfaceInfo->prefix << "/" << interfaceInfo->prefixLength;
            EV_INFO << endl;
        }
    }
}

void Ipv6NetworkConfigurator::dumpRoutes(Topology& topology)
{
    EV_INFO << "--- IPv6 Routes ---" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->staticRoutes.empty())
            continue;
        EV_INFO << node->module->getFullPath() << ":" << endl;
        for (auto *route : node->staticRoutes) {
            EV_INFO << "  " << route->getDestPrefix() << "/" << route->getPrefixLength()
                    << " via " << route->getNextHop();
            if (route->getInterface())
                EV_INFO << " dev " << route->getInterface()->getInterfaceName();
            EV_INFO << endl;
        }
    }
}

} // namespace inet
