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
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/VisualizerBase.h"

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

