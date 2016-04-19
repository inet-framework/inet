//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/switch/MACRelayUnit.h"
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/ipv4/IPv4.h"
#include "inet/visualizer/base/RouteVisualizerBase.h"

namespace inet {

namespace visualizer {

RouteVisualizerBase::Route::Route(const std::vector<int>& path) :
    path(path)
{
}

void RouteVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        subscriptionModule = *par("subscriptionModule").stringValue() == '\0' ? getSystemModule() : getModuleFromPar<cModule>(par("subscriptionModule"), this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
        subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromLowerSignal, this);
        subscriptionModule->subscribe(IMobility::mobilityStateChangedSignal, this);
        packetNameMatcher.setPattern(par("packetNameFilter"), false, true, true);
        if (*par("lineColor").stringValue() != '\0')
            lineColor = cFigure::Color(par("lineColor"));
        lineWidth = par("lineWidth");
        opacityHalfLife = par("opacityHalfLife");
    }
}

void RouteVisualizerBase::refreshDisplay() const
{
    auto now = simTime();
    std::vector<const Route *> removedRoutes;
    for (auto it : routes) {
        auto route = it.second;
        auto alpha = std::min(1.0, std::pow(2.0, -(now - route->lastUsage).dbl() / opacityHalfLife));
        if (alpha < 0.01)
            removedRoutes.push_back(route);
        else
            setAlpha(route, alpha);
    }
    for (auto route : removedRoutes) {
        auto sourceAndDestination = std::pair<int, int>(route->path.front(), route->path.back());
        const_cast<RouteVisualizerBase *>(this)->removeRoute(sourceAndDestination, route);
        delete route;
    }
}

const RouteVisualizerBase::Route *RouteVisualizerBase::createRoute(const std::vector<int>& path) const
{
    return new Route(path);
}

const RouteVisualizerBase::Route *RouteVisualizerBase::getRoute(std::pair<int, int> route, const std::vector<int>& path)
{
    auto range = routes.equal_range(route);
    for (auto it = range.first; it != range.second; it++)
        if (it->second->path == path)
            return it->second;
    return nullptr;
}

void RouteVisualizerBase::addRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    routes.insert(std::pair<std::pair<int, int>, const Route *>(sourceAndDestination, route));
    updateOffsets();
    updatePositions();
}

void RouteVisualizerBase::removeRoute(std::pair<int, int> sourceAndDestination, const Route *route)
{
    routes.erase(routes.find(sourceAndDestination));
    updateOffsets();
    updatePositions();
}

const std::vector<int> *RouteVisualizerBase::getIncompleteRoute(int treeId)
{
    auto it = incompleteRoutes.find(treeId);
    if (it == incompleteRoutes.end())
        return nullptr;
    else
        return &it->second;
}

void RouteVisualizerBase::addToIncompleteRoute(int treeId, cModule *module)
{
    incompleteRoutes[treeId].push_back(module->getId());
}

void RouteVisualizerBase::removeIncompleteRoute(int treeId)
{
    incompleteRoutes.erase(incompleteRoutes.find(treeId));
}

void RouteVisualizerBase::updateOffsets()
{
    numPaths.clear();
    for (auto it : routes) {
        auto route = it.second;
        int count = 0;
        int maxNumPath = 0;
        for (auto id : route->path) {
            int numPath = numPaths[id];
            if (numPath == maxNumPath)
                count++;
            else if (numPath > maxNumPath) {
                count = 1;
                maxNumPath = numPath;
            }
            numPaths[id] = numPath + 1;
        }
        route->offset = 3 * (maxNumPath + (count > 1 ? 1 : 0));
    }
}

void RouteVisualizerBase::updatePositions()
{
    for (auto it : numPaths) {
        auto id = it.first;
        auto node = getSimulation()->getModule(id);
        setPosition(node, getPosition(node));
    }
}

void RouteVisualizerBase::updateRoute(const std::vector<int>& path)
{
    auto key = std::pair<int, int>(path.front(), path.back());
    const Route *route = getRoute(key, path);
    if (route == nullptr) {
        route = createRoute(path);
        addRoute(key, route);
    }
    else
        route->lastUsage = simTime();
}

void RouteVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object DETAILS_ARG)
{
    if (signal == IMobility::mobilityStateChangedSignal) {
        auto mobility = dynamic_cast<IMobility *>(object);
        auto position = mobility->getCurrentPosition();
        auto module = check_and_cast<cModule *>(source);
        auto node = getContainingNode(module);
        setPosition(node, position);
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromUpperSignal) {
        if (dynamic_cast<IPv4 *>(source) != nullptr) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompleteRoute(treeId, getContainingNode(module));
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromLowerSignal) {
        if (dynamic_cast<IPv4 *>(source) != nullptr)
        {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getEncapsulatedPacket()->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompleteRoute(treeId, getContainingNode(module));
            }
        }
        else if (dynamic_cast<MACRelayUnit *>(source) != nullptr || dynamic_cast<Ieee8021dRelay *>(source) != nullptr)
        {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto encapsulatedPacket = packet->getEncapsulatedPacket()->getEncapsulatedPacket();
                if (encapsulatedPacket != nullptr) {
                    auto treeId = encapsulatedPacket->getTreeId();
                    auto module = check_and_cast<cModule *>(source);
                    addToIncompleteRoute(treeId, getContainingNode(module));
                }
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetSentToUpperSignal) {
        if (dynamic_cast<IPv4 *>(source) != nullptr) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto path = getIncompleteRoute(treeId);
                updateRoute(*path);
                removeIncompleteRoute(treeId);
            }
        }
    }
}

} // namespace visualizer

} // namespace inet

