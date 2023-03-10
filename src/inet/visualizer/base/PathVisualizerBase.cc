//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/PathVisualizerBase.h"

#include "inet/common/FlowTag.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

PathVisualizerBase::PathVisualization::PathVisualization(const char *label, const std::vector<int>& path) :
    ModulePath(path),
    label(label)
{
}

std::string PathVisualizerBase::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(pathVisualization->numPackets);
        case 'l':
            return pathVisualization->totalLength.str();
        case 'L':
            return pathVisualization->label;
        case 'n':
            return packet->getName();
        case 'c':
            return packet->getClassName();
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

void PathVisualizerBase::preDelete(cComponent *root)
{
    if (displayRoutes) {
        unsubscribe();
        removeAllPathVisualizations();
    }
}

void PathVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayRoutes = par("displayRoutes");
        nodeFilter.setPattern(par("nodeFilter"));
        packetFilter.setExpression(par("packetFilter").objectValue());
        lineColorSet.parseColors(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
        lineSmooth = par("lineSmooth");
        lineShift = par("lineShift");
        lineShiftMode = par("lineShiftMode");
        lineContactSpacing = par("lineContactSpacing");
        lineContactMode = par("lineContactMode");
        labelFormat.parseFormat(par("labelFormat"));
        labelFont = cFigure::parseFont(par("labelFont"));
        labelColorAsString = par("labelColor");
        if (!opp_isempty(labelColorAsString))
            labelColor = cFigure::Color(labelColorAsString);
        fadeOutMode = par("fadeOutMode");
        fadeOutTime = par("fadeOutTime");
        fadeOutAnimationSpeed = par("fadeOutAnimationSpeed");
        startPathSignal = registerSignal(par("startPathSignal"));
        extendPathSignal = registerSignal(par("extendPathSignal"));
        endPathSignal = registerSignal(par("endPathSignal"));
        if (displayRoutes)
            subscribe();
    }
}

void PathVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    removeAllPathVisualizations();
}

void PathVisualizerBase::refreshDisplay() const
{
    if (fadeOutTime > 0) {
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
}

void PathVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(startPathSignal, this);
    visualizationSubjectModule->subscribe(extendPathSignal, this);
    visualizationSubjectModule->subscribe(endPathSignal, this);
}

void PathVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(startPathSignal, this);
        visualizationSubjectModule->unsubscribe(extendPathSignal, this);
        visualizationSubjectModule->unsubscribe(endPathSignal, this);
    }
}

std::string PathVisualizerBase::getPathVisualizationText(const PathVisualization *pathVisualization, cPacket *packet) const
{
    DirectiveResolver directiveResolver(pathVisualization, packet);
    return labelFormat.formatString(&directiveResolver);
}

const PathVisualizerBase::PathVisualization *PathVisualizerBase::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    return new PathVisualization(label, path);
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

const std::vector<int> *PathVisualizerBase::getIncompletePath(const std::string& label, int chunkId)
{
    auto it = incompletePaths.find({ label, chunkId });
    return (it == incompletePaths.end()) ? nullptr : &it->second;
}

void PathVisualizerBase::addToIncompletePath(const std::string& label, int chunkId, cModule *module)
{
    auto& moduleIds = incompletePaths[{ label, chunkId }];
    auto moduleId = module->getId();
    if (moduleIds.size() == 0 || moduleIds[moduleIds.size() - 1] != moduleId)
        moduleIds.push_back(moduleId);
}

void PathVisualizerBase::removeIncompletePath(const std::string& label, int chunkId)
{
    incompletePaths.erase(incompletePaths.find({ label, chunkId }));
}

void PathVisualizerBase::refreshPathVisualization(const PathVisualization *pathVisualization, cPacket *packet)
{
    pathVisualization->lastUsageAnimationPosition = AnimationPosition();
}

void PathVisualizerBase::processPathStart(cModule *networkNode, const char *label, Packet *packet)
{
    mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int chunkId) {
        auto path = getIncompletePath(label, chunkId);
        if (path != nullptr)
            removeIncompletePath(label, chunkId);
        addToIncompletePath(label, chunkId, networkNode);
    });
}

void PathVisualizerBase::processPathElement(cModule *networkNode, const char *label, Packet *packet)
{
    mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int chunkId) {
        auto path = getIncompletePath(label, chunkId);
        if (path != nullptr)
            addToIncompletePath(label, chunkId, networkNode);
    });
}

void PathVisualizerBase::processPathEnd(cModule *networkNode, const char *label, Packet *packet)
{
    std::set<const PathVisualization *> updatedVisualizations;
    mapChunks(packet->peekAt(b(0), packet->getTotalLength()), [&] (const Ptr<const Chunk>& chunk, int chunkId) {
        auto path = getIncompletePath(label, chunkId);
        if (path != nullptr) {
            addToIncompletePath(label, chunkId, networkNode);
            if (path->size() > 1) {
                auto pathVisualization = getPathVisualization(*path);
                if (pathVisualization == nullptr) {
                    pathVisualization = createPathVisualization(label, *path, packet);
                    addPathVisualization(pathVisualization);
                }
                pathVisualization->totalLength += chunk->getChunkLength();
                updatedVisualizations.insert(pathVisualization);
            }
            removeIncompletePath(label, chunkId);
        }
    });
    for (auto pathVisualization : updatedVisualizations) {
        pathVisualization->numPackets++;
        refreshPathVisualization(pathVisualization, packet);
    }
}

void PathVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == startPathSignal) {
        auto module = check_and_cast<cModule *>(source);
        if (isPathStart(module)) {
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<Packet *>(object);
            auto label = details != nullptr ? details->getName() : "";
            if (nodeFilter.matches(networkNode) && packetFilter.matches(packet))
                processPathStart(networkNode, label, packet);
        }
    }
    else if (signal == extendPathSignal) {
        auto module = check_and_cast<cModule *>(source);
        if (isPathElement(module)) {
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<Packet *>(object);
            auto label = details != nullptr ? details->getName() : "";
            // NOTE: nodeFilter is intentionally not applied here, because it's only important at the end points
            if (packetFilter.matches(packet))
                processPathElement(networkNode, label, packet);
        }
    }
    else if (signal == endPathSignal) {
        auto module = check_and_cast<cModule *>(source);
        if (isPathEnd(module)) {
            auto networkNode = getContainingNode(module);
            auto packet = check_and_cast<Packet *>(object);
            auto label = details != nullptr ? details->getName() : "";
            if (nodeFilter.matches(networkNode) && packetFilter.matches(packet))
                processPathEnd(networkNode, label, packet);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

