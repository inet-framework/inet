//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif

namespace inet {

namespace visualizer {

RoutingTableVisualizerBase::RouteVisualization::RouteVisualization(const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
    ModuleLine(nodeModuleId, nextHopModuleId),
    route(route)
{
}

RoutingTableVisualizerBase::MulticastRouteVisualization::MulticastRouteVisualization(const Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId) :
    ModuleLine(nodeModuleId, nextHopModuleId),
    route(route)
{
}

std::string RoutingTableVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'm':
            return route->getNetmask().isUnspecified() ? "*" : std::to_string(route->getNetmask().getNetmaskLength());
        case 'g':
            return route->getGateway().isUnspecified() ? "*" : route->getGateway().str();
        case 'd':
            return route->getDestination().isUnspecified() ? "*" : route->getDestination().str();
        case 'e':
            return std::to_string(route->getMetric());
        case 'n':
            return route->getInterface()->getInterfaceName();
        case 'i':
            return route->str();
        case 's':
            return route->str();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

std::string RoutingTableVisualizerBase::MulticastDirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'e':
            return std::to_string(route->getMetric());
        case 'i':
            return route->str();
        case 's':
            return route->str();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void RoutingTableVisualizerBase::preDelete(cComponent *root)
{
    if (displayRoutingTables) {
        unsubscribe();
        removeAllRouteVisualizations();
    }
}

void RoutingTableVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayRoutingTables = par("displayRoutingTables");
        displayRoutesIndividually = par("displayRoutesIndividually");
        displayLabels = par("displayLabels");
        destinationFilter.setPattern(par("destinationFilter"));
        multicastSourceNodeFilter.setPattern(par("multicastSourceNodeFilter"));
        multicastSourceAddressFilter.setPattern(par("multicastSourceAddressFilter"), false, true, true);
        multicastGroupFilter.setPattern(par("multicastGroupFilter"), false, true, true);
        nodeFilter.setPattern(par("nodeFilter"));
        lineColor = cFigure::Color(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
        lineShift = par("lineShift");
        lineShiftMode = par("lineShiftMode");
        lineContactSpacing = par("lineContactSpacing");
        lineContactMode = par("lineContactMode");
        labelFormat.parseFormat(par("labelFormat"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        if (displayRoutingTables)
            subscribe();
    }
}

void RoutingTableVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "destinationFilter"))
        destinationFilter.setPattern(par("destinationFilter"));
    else if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "multicastSourceNodeFilter"))
        multicastSourceNodeFilter.setPattern(par("multicastSourceNodeFilter"));
    else if (!strcmp(name, "labelFormat"))
        labelFormat.parseFormat(par("labelFormat"));
    allRoutingTableVisualizationsAreInvalid = true;
}

void RoutingTableVisualizerBase::refreshDisplay() const
{
    if (displayRoutingTables)
        // KLUDGE TODO
        const_cast<RoutingTableVisualizerBase *>(this)->updateRouteVisualizations();
}

void RoutingTableVisualizerBase::updateRouteVisualizations()
{
    if (allRoutingTableVisualizationsAreInvalid) {
        updateAllRouteVisualizations();
        allRoutingTableVisualizationsAreInvalid = false;
    }
    else
        for (auto routingTable : invalidRoutingTableVisualizations)
            updateRouteVisualizations(routingTable);
    invalidRoutingTableVisualizations.clear();
}

void RoutingTableVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(routeAddedSignal, this);
    visualizationSubjectModule->subscribe(routeDeletedSignal, this);
    visualizationSubjectModule->subscribe(routeChangedSignal, this);
    visualizationSubjectModule->subscribe(mrouteAddedSignal, this);
    visualizationSubjectModule->subscribe(mrouteDeletedSignal, this);
    visualizationSubjectModule->subscribe(mrouteChangedSignal, this);
    visualizationSubjectModule->subscribe(interfaceIpv4ConfigChangedSignal, this);
}

void RoutingTableVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(routeAddedSignal, this);
        visualizationSubjectModule->unsubscribe(routeDeletedSignal, this);
        visualizationSubjectModule->unsubscribe(routeChangedSignal, this);
        visualizationSubjectModule->unsubscribe(mrouteAddedSignal, this);
        visualizationSubjectModule->unsubscribe(mrouteDeletedSignal, this);
        visualizationSubjectModule->unsubscribe(mrouteChangedSignal, this);
        visualizationSubjectModule->unsubscribe(interfaceIpv4ConfigChangedSignal, this);
    }
}

void RoutingTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == routeAddedSignal || signal == routeDeletedSignal || signal == routeChangedSignal ||
        signal == mrouteAddedSignal || signal == mrouteDeletedSignal || signal == mrouteChangedSignal)
    {
        auto routingTable = check_and_cast<IIpv4RoutingTable *>(source);
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode)) {
            removeRouteVisualizations(routingTable);
            invalidRoutingTableVisualizations.insert(routingTable);
        }
    }
    else if (signal == interfaceIpv4ConfigChangedSignal)
        allRoutingTableVisualizationsAreInvalid = true;
    else
        throw cRuntimeError("Unknown signal");
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableVisualizerBase::getRouteVisualization(Ipv4Route *route, int nodeModuleId, int nextHopModuleId)
{
    Ipv4Address routerId;
    if (route != nullptr)
        routerId = route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, nodeModuleId, nextHopModuleId);
    auto it = routeVisualizations.find(key);
    return it == routeVisualizations.end() ? nullptr : it->second;
}

void RoutingTableVisualizerBase::addRouteVisualization(const RouteVisualization *routeVisualization)
{
    Ipv4Address routerId;
    if (routeVisualization->route != nullptr)
        routerId = routeVisualization->route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    routeVisualizations[key] = routeVisualization;
}

void RoutingTableVisualizerBase::removeRouteVisualization(const RouteVisualization *routeVisualization)
{
    Ipv4Address routerId;
    if (routeVisualization->route != nullptr)
        routerId = routeVisualization->route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    routeVisualizations.erase(routeVisualizations.find(key));
}

const RoutingTableVisualizerBase::MulticastRouteVisualization *RoutingTableVisualizerBase::getMulticastRouteVisualization(Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId)
{
    Ipv4Address routerId;
    if (route != nullptr)
        routerId = route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, nodeModuleId, nextHopModuleId);
    auto it = multicastRouteVisualizations.find(key);
    return it == multicastRouteVisualizations.end() ? nullptr : it->second;
}

void RoutingTableVisualizerBase::addMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization)
{
    Ipv4Address routerId;
    if (routeVisualization->route != nullptr)
        routerId = routeVisualization->route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    multicastRouteVisualizations[key] = routeVisualization;
}

void RoutingTableVisualizerBase::removeMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization)
{
    Ipv4Address routerId;
    if (routeVisualization->route != nullptr)
        routerId = routeVisualization->route->getRoutingTable()->getRouterId();
    auto key = std::make_tuple(routerId, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    multicastRouteVisualizations.erase(multicastRouteVisualizations.find(key));
}

std::vector<Ipv4Address> RoutingTableVisualizerBase::getDestinations()
{
    L3AddressResolver addressResolver;
    std::vector<Ipv4Address> destinations;
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && destinationFilter.matches(networkNode)) {
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                    auto interface = interfaceTable->getInterface(i);
                    auto address = interface->getIpv4Address();
                    if (!address.isUnspecified())
                        destinations.push_back(address);
                }
            }
        }
    }
    return destinations;
}

std::vector<Ipv4Address> RoutingTableVisualizerBase::getMulticastSources()
{
    L3AddressResolver addressResolver;
    std::vector<Ipv4Address> multicastSources;
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && multicastSourceNodeFilter.matches(networkNode)) {
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                    auto interface = interfaceTable->getInterface(i);
                    cMatchableString matchableAddress(interface->getIpv4Address().str().c_str());
                    if (!interface->isLoopback() && multicastSourceAddressFilter.matches(&matchableAddress)) {
                        auto address = interface->getIpv4Address();
                        if (!address.isUnspecified())
                            multicastSources.push_back(address);
                    }
                }
            }
        }
    }
    return multicastSources;
}

std::vector<Ipv4Address> RoutingTableVisualizerBase::getMulticastGroups()
{
    L3AddressResolver addressResolver;
    std::set<Ipv4Address> multicastGroups;
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode)) {
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                    auto interface = interfaceTable->getInterface(i);
#ifdef INET_WITH_IPv4
                    auto protocolData = interface->findProtocolData<Ipv4InterfaceData>();
                    if (protocolData != nullptr) {
                        for (int j = 0; j < protocolData->getNumOfJoinedMulticastGroups(); j++) {
                            auto multicastGroup = protocolData->getJoinedMulticastGroup(j);
                            cMatchableString matchableGroup(multicastGroup.str().c_str());
                            if (multicastGroupFilter.matches(&matchableGroup))
                                multicastGroups.insert(multicastGroup);
                        }
                    }
#endif
                }
            }
        }
    }
    return std::vector<Ipv4Address>(multicastGroups.begin(), multicastGroups.end());
}

void RoutingTableVisualizerBase::addRouteVisualizations(IIpv4RoutingTable *routingTable)
{
    L3AddressResolver addressResolver;
    auto node = getContainingNode(check_and_cast<cModule *>(routingTable));
    for (auto destination : getDestinations()) {
        if (!routingTable->isLocalAddress(destination)) {
            auto route = routingTable->findBestMatchingRoute(destination);
            if (route != nullptr) {
                auto gateway = route->getGateway();
                auto nextHop = addressResolver.findHostWithAddress(gateway.isUnspecified() ? destination : gateway);
                if (nextHop != nullptr) {
                    auto routeVisualization = getRouteVisualization(displayRoutesIndividually ? route : nullptr, node->getId(), nextHop->getId());
                    if (routeVisualization == nullptr)
                        addRouteVisualization(createRouteVisualization(displayRoutesIndividually ? route : nullptr, node, nextHop));
                    else {
                        routeVisualization->numRoutes++;
                        refreshRouteVisualization(routeVisualization);
                    }
                }
            }
        }
    }
    auto multicastSources = getMulticastSources();
    auto multicastGroups = getMulticastGroups();
    for (auto source : multicastSources) {
        for (auto group : multicastGroups) {
            auto route = const_cast<Ipv4MulticastRoute *>(routingTable->findBestMatchingMulticastRoute(source, group));
            if (route != nullptr) {
                for (unsigned int i = 0; i < route->getNumOutInterfaces(); i++) {
                    auto outInterface = route->getOutInterface(i);
                    auto interface = const_cast<NetworkInterface *>(outInterface->getInterface());
                    auto channel = interface->getTxTransmissionChannel();
                    if (channel != nullptr) {
                        auto endGate = channel->getSourceGate()->getPathEndGate();
                        if (endGate != nullptr) {
                            auto nextHop = getContainingNode(endGate->getOwnerModule());
                            if (nextHop != nullptr) {
                                auto routeVisualization = getMulticastRouteVisualization(displayRoutesIndividually ? route : nullptr, node->getId(), nextHop->getId());
                                if (routeVisualization == nullptr)
                                    addMulticastRouteVisualization(createMulticastRouteVisualization(displayRoutesIndividually ? route : nullptr, node, nextHop));
                                else {
                                    routeVisualization->numRoutes++;
                                    refreshMulticastRouteVisualization(routeVisualization);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void RoutingTableVisualizerBase::removeRouteVisualizations(IIpv4RoutingTable *routingTable)
{
    auto networkNode = getContainingNode(check_and_cast<cModule *>(routingTable));
    std::vector<const RouteVisualization *> removedRouteVisualizations;
    for (auto it : routeVisualizations)
        if (std::get<1>(it.first) == networkNode->getId() && it.second)
            removedRouteVisualizations.push_back(it.second);
    for (auto it : removedRouteVisualizations) {
        removeRouteVisualization(it);
        delete it;
    }
    std::vector<const MulticastRouteVisualization *> removedMulticastRouteVisualizations;
    for (auto it : multicastRouteVisualizations)
        if (std::get<1>(it.first) == networkNode->getId() && it.second)
            removedMulticastRouteVisualizations.push_back(it.second);
    for (auto it : removedMulticastRouteVisualizations) {
        removeMulticastRouteVisualization(it);
        delete it;
    }
}

void RoutingTableVisualizerBase::addAllRouteVisualizations()
{
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
            L3AddressResolver addressResolver;
            auto routingTable = addressResolver.findIpv4RoutingTableOf(networkNode);
            if (routingTable != nullptr)
                addRouteVisualizations(routingTable);
        }
    }
}

void RoutingTableVisualizerBase::updateRouteVisualizations(IIpv4RoutingTable *routingTable)
{
    removeRouteVisualizations(routingTable);
    addRouteVisualizations(routingTable);
}

void RoutingTableVisualizerBase::removeAllRouteVisualizations()
{
    std::vector<const RouteVisualization *> removedRouteVisualizations;
    for (auto it : routeVisualizations)
        removedRouteVisualizations.push_back(it.second);
    for (auto it : removedRouteVisualizations) {
        removeRouteVisualization(it);
        delete it;
    }
    std::vector<const MulticastRouteVisualization *> removedMulticastRouteVisualizations;
    for (auto it : multicastRouteVisualizations)
        removedMulticastRouteVisualizations.push_back(it.second);
    for (auto it : removedMulticastRouteVisualizations) {
        removeMulticastRouteVisualization(it);
        delete it;
    }
}

void RoutingTableVisualizerBase::updateAllRouteVisualizations()
{
    removeAllRouteVisualizations();
    addAllRouteVisualizations();
}

std::string RoutingTableVisualizerBase::getRouteVisualizationText(const Ipv4Route *route) const
{
    DirectiveResolver directiveResolver(route);
    return labelFormat.formatString(&directiveResolver);
}

std::string RoutingTableVisualizerBase::getMulticastRouteVisualizationText(const Ipv4MulticastRoute *route) const
{
    MulticastDirectiveResolver directiveResolver(route);
    return labelFormat.formatString(&directiveResolver);
}

} // namespace visualizer

} // namespace inet

