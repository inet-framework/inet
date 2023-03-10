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
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

namespace visualizer {

RoutingTableVisualizerBase::RouteVisualization::RouteVisualization(const Ipv4Route *route, int nodeModuleId, int nextHopModuleId) :
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
    else if (!strcmp(name, "labelFormat"))
        labelFormat.parseFormat(par("labelFormat"));
    updateAllRouteVisualizations();
}

void RoutingTableVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(routeAddedSignal, this);
    visualizationSubjectModule->subscribe(routeDeletedSignal, this);
    visualizationSubjectModule->subscribe(routeChangedSignal, this);
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
        visualizationSubjectModule->unsubscribe(interfaceIpv4ConfigChangedSignal, this);
    }
}

void RoutingTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == routeAddedSignal || signal == routeDeletedSignal || signal == routeChangedSignal) {
        auto routingTable = check_and_cast<IIpv4RoutingTable *>(source);
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode))
            updateRouteVisualizations(routingTable);
    }
    else if (signal == interfaceIpv4ConfigChangedSignal)
        updateAllRouteVisualizations();
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
}

void RoutingTableVisualizerBase::updateRouteVisualizations(IIpv4RoutingTable *routingTable)
{
    removeRouteVisualizations(routingTable);
    addRouteVisualizations(routingTable);
}

void RoutingTableVisualizerBase::updateAllRouteVisualizations()
{
    removeAllRouteVisualizations();
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

std::string RoutingTableVisualizerBase::getRouteVisualizationText(const Ipv4Route *route) const
{
    DirectiveResolver directiveResolver(route);
    return labelFormat.formatString(&directiveResolver);
}

} // namespace visualizer

} // namespace inet

