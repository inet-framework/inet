//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

RoutingTableVisualizerBase::RouteVisualization::RouteVisualization(const IPv4Route *route, int nodeModuleId, int nextHopModuleId) :
    ModuleLine(nodeModuleId, nextHopModuleId),
    route(route)
{
}

const char *RoutingTableVisualizerBase::DirectiveResolver::resolveDirective(char directive)
{
    switch (directive) {
        case 'm':
            result = route->getNetmask().isUnspecified() ? "*" : std::to_string(route->getNetmask().getNetmaskLength());
            break;
        case 'g':
            result = route->getGateway().isUnspecified() ? "*" : route->getGateway().str();
            break;
        case 'd':
            result = route->getDestination().isUnspecified() ? "*" : route->getDestination().str();
            break;
        case 'n':
            result = route->getInterface()->getName();
            break;
        case 'i':
            result = route->str();
            break;
        case 's':
            result = route->str();
            break;
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
    return result.c_str();
}

RoutingTableVisualizerBase::~RoutingTableVisualizerBase()
{
    if (displayRoutingTables)
        unsubscribe();
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
        lineManager = LineManager::getLineManager(visualizerTargetModule->getCanvas());
        labelFormat.parseFormat(par("labelFormat"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColor = cFigure::Color(par("labelColor"));
        if (displayRoutingTables)
            subscribe();
    }
}

void RoutingTableVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "destinationFilter"))
            destinationFilter.setPattern(par("destinationFilter"));
        else if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "labelFormat"))
            labelFormat.parseFormat(par("labelFormat"));
        updateAllRouteVisualizations();
    }
}

void RoutingTableVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(NF_ROUTE_ADDED, this);
    subscriptionModule->subscribe(NF_ROUTE_DELETED, this);
    subscriptionModule->subscribe(NF_ROUTE_CHANGED, this);
    subscriptionModule->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
}

void RoutingTableVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(NF_ROUTE_ADDED, this);
        subscriptionModule->unsubscribe(NF_ROUTE_DELETED, this);
        subscriptionModule->unsubscribe(NF_ROUTE_CHANGED, this);
        subscriptionModule->unsubscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
}

void RoutingTableVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == NF_ROUTE_ADDED || signal == NF_ROUTE_DELETED || signal == NF_ROUTE_CHANGED) {
        auto routingTable = check_and_cast<IIPv4RoutingTable *>(source);
        auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
        if (nodeFilter.matches(networkNode))
            updateRouteVisualizations(routingTable);
    }
    else if (signal == NF_INTERFACE_IPv4CONFIG_CHANGED)
        updateAllRouteVisualizations();
    else
        throw cRuntimeError("Unknown signal");
}

const RoutingTableVisualizerBase::RouteVisualization *RoutingTableVisualizerBase::getRouteVisualization(IPv4Route *route, int nodeModuleId, int nextHopModuleId)
{
    auto key = std::make_tuple(route, nodeModuleId, nextHopModuleId);
    auto it = routeVisualizations.find(key);
    return it == routeVisualizations.end() ? nullptr : it->second;
}

void RoutingTableVisualizerBase::addRouteVisualization(const RouteVisualization *routeVisualization)
{
    auto key = std::make_tuple(routeVisualization->route, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    routeVisualizations[key] = routeVisualization;
}

void RoutingTableVisualizerBase::removeRouteVisualization(const RouteVisualization *routeVisualization)
{
    auto key = std::make_tuple(routeVisualization->route, routeVisualization->sourceModuleId, routeVisualization->destinationModuleId);
    routeVisualizations.erase(routeVisualizations.find(key));
}

std::vector<IPv4Address> RoutingTableVisualizerBase::getDestinations()
{
    L3AddressResolver addressResolver;
    std::vector<IPv4Address> destinations;
    for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && destinationFilter.matches(networkNode)) {
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                auto interface = interfaceTable->getInterface(i);
                if (interface->ipv4Data() != nullptr) {
                    auto address = interface->ipv4Data()->getIPAddress();
                    if (!address.isUnspecified())
                        destinations.push_back(address);
                }
            }
        }
    }
    return destinations;
}

void RoutingTableVisualizerBase::addRouteVisualizations(IIPv4RoutingTable *routingTable)
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

void RoutingTableVisualizerBase::removeRouteVisualizations(IIPv4RoutingTable *routingTable)
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

void RoutingTableVisualizerBase::updateRouteVisualizations(IIPv4RoutingTable *routingTable)
{
    removeRouteVisualizations(routingTable);
    addRouteVisualizations(routingTable);
}

void RoutingTableVisualizerBase::updateAllRouteVisualizations()
{
    removeAllRouteVisualizations();
    for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
            L3AddressResolver addressResolver;
            auto routingTable = addressResolver.findIPv4RoutingTableOf(networkNode);
            if (routingTable != nullptr)
                addRouteVisualizations(routingTable);
        }
    }
}

std::string RoutingTableVisualizerBase::getRouteVisualizationText(const IPv4Route *route) const
{
    DirectiveResolver directiveResolver(route);
    return labelFormat.formatString(&directiveResolver);
}

} // namespace visualizer

} // namespace inet

