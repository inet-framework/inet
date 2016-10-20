//
// Copyright (C) 2016 OpenSim Ltd.
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

PathVisualizerBase::Path::Path(const std::vector<int>& path) :
    moduleIds(path)
{
}

void PathVisualizerBase::initialize(int stage)
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

void PathVisualizerBase::refreshDisplay() const
{
    auto now = simTime();
    std::vector<const Path *> removedPaths;
    for (auto it : paths) {
        auto path = it.second;
        auto alpha = std::min(1.0, std::pow(2.0, -(now - path->lastUsage).dbl() / opacityHalfLife));
        if (alpha < 0.01)
            removedPaths.push_back(path);
        else
            setAlpha(path, alpha);
    }
    for (auto path : removedPaths) {
        auto sourceAndDestination = std::pair<int, int>(path->moduleIds.front(), path->moduleIds.back());
        const_cast<PathVisualizerBase *>(this)->removePath(sourceAndDestination, path);
        delete path;
    }
}

const PathVisualizerBase::Path *PathVisualizerBase::createPath(const std::vector<int>& path) const
{
    return new Path(path);
}

const PathVisualizerBase::Path *PathVisualizerBase::getPath(std::pair<int, int> sourceAndDestination, const std::vector<int>& path)
{
    auto range = paths.equal_range(sourceAndDestination);
    for (auto it = range.first; it != range.second; it++)
        if (it->second->moduleIds == path)
            return it->second;
    return nullptr;
}

void PathVisualizerBase::addPath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    paths.insert(std::pair<std::pair<int, int>, const Path *>(sourceAndDestination, path));
    updateOffsets();
    updatePositions();
}

void PathVisualizerBase::removePath(std::pair<int, int> sourceAndDestination, const Path *path)
{
    paths.erase(paths.find(sourceAndDestination));
    updateOffsets();
    updatePositions();
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

void PathVisualizerBase::updateOffsets()
{
    numPaths.clear();
    for (auto it : paths) {
        auto path = it.second;
        int count = 0;
        int maxNumPath = 0;
        for (auto id : path->moduleIds) {
            int numPath = numPaths[id];
            if (numPath == maxNumPath)
                count++;
            else if (numPath > maxNumPath) {
                count = 1;
                maxNumPath = numPath;
            }
            numPaths[id] = numPath + 1;
        }
        path->offset = 3 * (maxNumPath + (count > 1 ? 1 : 0));
    }
}

void PathVisualizerBase::updatePositions()
{
    for (auto it : numPaths) {
        auto id = it.first;
        auto node = getSimulation()->getModule(id);
        setPosition(node, getPosition(node));
    }
}

void PathVisualizerBase::updatePath(const std::vector<int>& moduleIds)
{
    auto key = std::pair<int, int>(moduleIds.front(), moduleIds.back());
    const Path *path = getPath(key, moduleIds);
    if (path == nullptr) {
        path = createPath(moduleIds);
        addPath(key, path);
    }
    else
        path->lastUsage = simTime();
}

void PathVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == IMobility::mobilityStateChangedSignal) {
        auto mobility = dynamic_cast<IMobility *>(object);
        auto position = mobility->getCurrentPosition();
        auto module = check_and_cast<cModule *>(source);
        auto node = getContainingNode(module);
        setPosition(node, position);
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromUpperSignal) {
        if (isPathEnd(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompletePath(treeId, getContainingNode(module));
            }
        }
    }
    else if (signal == LayeredProtocolBase::packetReceivedFromLowerSignal) {
        if (isPathEnd(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getEncapsulatedPacket()->getTreeId();
                auto module = check_and_cast<cModule *>(source);
                addToIncompletePath(treeId, getContainingNode(module));
            }
        }
        else if (isPathElement(static_cast<cModule *>(source))) {
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
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
            auto packet = check_and_cast<cPacket *>(object);
            if (packetNameMatcher.matches(packet->getFullName())) {
                auto treeId = packet->getTreeId();
                auto path = getIncompletePath(treeId);
                updatePath(*path);
                removeIncompletePath(treeId);
            }
        }
    }
}

} // namespace visualizer

} // namespace inet

