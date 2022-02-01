//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/VisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace visualizer {

void VisualizerBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        const char *path = par("visualizationTargetModule");
        visualizationTargetModule = getModuleByPath(path);
        if (visualizationTargetModule == nullptr)
            throw cRuntimeError("Module not found on path '%s' defined by par 'visualizationTargetModule'", path);
        visualizationSubjectModule = getModuleByPath(path);
        if (visualizationSubjectModule == nullptr)
            throw cRuntimeError("Module not found on path '%s' defined by par 'visualizationSubjectModule'", path);
        tags = par("tags");
    }
}

Coord VisualizerBase::getPosition(const cModule *networkNode) const
{
    auto mobility = networkNode->getSubmodule("mobility");
    if (mobility == nullptr) {
        auto bounds = getSimulation()->getEnvir()->getSubmoduleBounds(networkNode);
        auto center = bounds.getCenter();
        return Coord(center.x, center.y, 0);
    }
    else
        return check_and_cast<IMobility *>(mobility)->getCurrentPosition();
}

Coord VisualizerBase::getContactPosition(const cModule *networkNode, const Coord& fromPosition, const char *contactMode, double contactSpacing) const
{
    double zoomLevel = getEnvir()->getZoomLevel(networkNode->getParentModule());
    if (std::isnan(zoomLevel))
        zoomLevel = 1.0;

    if (!strcmp(contactMode, "circular")) {
        auto bounds = getSimulation()->getEnvir()->getSubmoduleBounds(networkNode);
        auto size = bounds.getSize();
        auto radius = std::sqrt(size.x * size.x + size.y * size.y) / 2;
        auto position = getPosition(networkNode);
        auto direction = fromPosition - position;
        direction.normalize();
        direction *= radius + contactSpacing;
        direction /= zoomLevel;
        return position + direction;
    }
    else if (!strcmp(contactMode, "rectangular")) {
        auto position = getPosition(networkNode);
        auto bounds = getSimulation()->getEnvir()->getSubmoduleBounds(networkNode);
        auto size = bounds.getSize();
        Cuboid networkNodeShape(Coord(size.x + 2 * contactSpacing / zoomLevel, size.y + 2 * contactSpacing / zoomLevel, 1));
        Coord intersection1, intersection2, normal1, normal2;
        networkNodeShape.computeIntersection(LineSegment(Coord::ZERO, fromPosition - position), intersection1, intersection2, normal1, normal2);
        return position + intersection2;
    }
    else
        throw cRuntimeError("Unknown contact mode: %s", contactMode);
}

Quaternion VisualizerBase::getOrientation(const cModule *networkNode) const
{
    auto mobility = networkNode->getSubmodule("mobility");
    if (mobility == nullptr)
        return Quaternion::IDENTITY;
    else
        return check_and_cast<IMobility *>(mobility)->getCurrentAngularPosition();
}

void VisualizerBase::mapChunks(const Ptr<const Chunk>& chunk, const std::function<void(const Ptr<const Chunk>&, int)>& thunk) const
{
    if (chunk->getChunkType() == Chunk::CT_SEQUENCE) {
        for (const auto& elementChunk : staticPtrCast<const SequenceChunk>(chunk)->getChunks())
            mapChunks(elementChunk, thunk);
    }
    else if (chunk->getChunkType() == Chunk::CT_SLICE)
        thunk(chunk, staticPtrCast<const SliceChunk>(chunk)->getChunk()->getChunkId());
    else
        thunk(chunk, chunk->getChunkId());
}

} // namespace visualizer

} // namespace inet

