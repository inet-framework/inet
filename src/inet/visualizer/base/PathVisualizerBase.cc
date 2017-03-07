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

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

PathVisualizerBase::PathVisualization::PathVisualization(const std::vector<int>& path) :
    ModulePath(path)
{
}

PathVisualizerBase::~PathVisualizerBase()
{
    if (displayRoutes)
        unsubscribe();
}

void PathVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayRoutes = par("displayRoutes");
        nodeFilter.setPattern(par("nodeFilter"));
        packetFilter.setPattern(par("packetFilter"));
        lineColorSet.parseColors(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
        lineShift = par("lineShift");
        lineShiftMode = par("lineShiftMode");
        lineContactSpacing = par("lineContactSpacing");
        lineContactMode = par("lineContactMode");
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        lineManager = LineManager::getLineManager(visualizerTargetModule->getCanvas());
        if (displayRoutes)
            subscribe();
    }
}

void PathVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
        removeAllPathVisualizations();
    }
}

void PathVisualizerBase::refreshDisplay() const
{
    AnimationPosition currentAnimationPosition;
    std::vector<const PathVisualization *> removedPathVisualizations;
    for (auto it : pathVisualizations) {
        auto pathVisualization = it.second;
        double delta;
        if (!strcmp(fadeOutMode, "simulationTime"))
            delta = (currentAnimationPosition.getSimulationTime() - pathVisualization->lastUsageAnimationPosition.getSimulationTime()).dbl();
        else if (!strcmp(fadeOutMode, "animationTime"))
            delta = currentAnimationPosition.getAnimationTime() - pathVisualization->lastUsageAnimationPosition.getAnimationTime();
        else if (!strcmp(fadeOutMode, "realTime"))
            delta = currentAnimationPosition.getRealTime() - pathVisualization->lastUsageAnimationPosition.getRealTime();
        else
            throw cRuntimeError("Unknown fadeOutMode: %s", fadeOutMode);
        if (delta > fadeOutTime)
            removedPathVisualizations.push_back(pathVisualization);
        else
            setAlpha(pathVisualization, 1 - delta / fadeOutTime);
    }
    for (auto path : removedPathVisualizations) {
        const_cast<PathVisualizerBase *>(this)->removePathVisualization(path);
        delete path;
    }
}

void PathVisualizerBase::subscribe()
{
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
    subscriptionModule->subscribe(LayeredProtocolBase::packetReceivedFromLowerSignal, this);
}

void PathVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto subscriptionModule = getModuleFromPar<cModule>(par("subscriptionModule"), this, false);
    if (subscriptionModule != nullptr) {
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetSentToUpperSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetReceivedFromUpperSignal, this);
        subscriptionModule->unsubscribe(LayeredProtocolBase::packetReceivedFromLowerSignal, this);
    }
}

const PathVisualizerBase::PathVisualization *PathVisualizerBase::createPathVisualization(const std::vector<int>& path) const
{
    return new PathVisualization(path);
}

const PathVisualizerBase::PathVisualization *PathVisualizerBase::getPathVisualization(const std::vector<int>& path)
{
    auto key = std::pair<int, int>(path.front(), path.back());
    auto range = pathVisualizations.equal_range(key);
    for (auto it = range.first; it != range.second; it++)
        if (it->second->moduleIds == path)
            return it->second;
    return nullptr;
}

void PathVisualizerBase::addPathVisualization(const PathVisualization *pathVisualization)
{
    auto sourceAndDestination = std::pair<int, int>(pathVisualization->moduleIds.front(), pathVisualization->moduleIds.back());
    pathVisualizations.insert(std::pair<std::pair<int, int>, const PathVisualization *>(sourceAndDestination, pathVisualization));
}

void PathVisualizerBase::removePathVisualization(const PathVisualization *pathVisualization)
{
    auto sourceAndDestination = std::pair<int, int>(pathVisualization->moduleIds.front(), pathVisualization->moduleIds.back());
    auto range = pathVisualizations.equal_range(sourceAndDestination);
    for (auto it = range.first; it != range.second; it++) {
        if (it->second == pathVisualization) {
            pathVisualizations.erase(it);
            break;
        }
    }
}

void PathVisualizerBase::removeAllPathVisualizations()
{
    incompletePaths.clear();
    numPaths.clear();
    std::vector<const PathVisualization *> removedPathVisualizations;
    for (auto it : pathVisualizations)
        removedPathVisualizations.push_back(it.second);
    for (auto it : removedPathVisualizations) {
        removePathVisualization(it);
        delete it;
    }
}

const std::vector<int> *PathVisualizerBase::getIncompletePath(int treeId)
{
    auto it = incompletePaths.find(treeId);
    if (it == incompletePaths.end())
        return nullptr;
    else
        return &it->second;
}

void PathVisualizerBase::addToIncompletePath(int treeId, cModule *module)
{
    incompletePaths[treeId].push_back(module->getId());
}

void PathVisualizerBase::removeIncompletePath(int treeId)
{
    incompletePaths.erase(incompletePaths.find(treeId));
}

void PathVisualizerBase::updatePath(const std::vector<int>& moduleIds)
{
    const PathVisualization *pathVisualization = getPathVisualization(moduleIds);
    if (pathVisualization == nullptr) {
        pathVisualization = createPathVisualization(moduleIds);
        addPathVisualization(pathVisualization);
    }
    else
        pathVisualization->lastUsageAnimationPosition = AnimationPosition();
}

void PathVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == LayeredProtocolBase::packetReceivedFromUpperSignal) {
        if (isPathEnd(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<cPacket *>(object);
            if (nodeFilter.matches(networkNode) && packetFilter.matches(packet)) {
                auto treeId = packet->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompletePath(treeId, getContainingNode(module));
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromLowerSignal) {
        if (isPathEnd(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetFilter.matches(packet)) {
                auto treeId = packet->getEncapsulatedPacket()->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompletePath(treeId, getContainingNode(module));
            }
        }
        else if (isPathElement(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetFilter.matches(packet)) {
                auto encapsulatedPacket = packet->getEncapsulatedPacket()->getEncapsulatedPacket();
                if (encapsulatedPacket != nullptr) {
                    auto treeId = encapsulatedPacket->getTreeId();
                    auto module = check_and_cast<cModule *>(source);
                    addToIncompletePath(treeId, getContainingNode(module));
                }
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetSentToUpperSignal) {
        if (isPathEnd(static_cast<cModule *>(source))) {
            auto module = check_and_cast<cModule *>(source);
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<cPacket *>(object);
            if (nodeFilter.matches(networkNode) && packetFilter.matches(packet)) {
                auto treeId = packet->getTreeId();
                auto path = getIncompletePath(treeId);
                updatePath(*path);
                removeIncompletePath(treeId);
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

