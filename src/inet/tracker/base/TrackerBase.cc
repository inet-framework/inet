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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/tracker/base/TrackerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

namespace tracker {

void TrackerBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *path = par("trackingTargetModule");
        trackingTargetModule = getModuleByPath(path);
        if (trackingTargetModule == nullptr)
            throw cRuntimeError("Module not found on path '%s' defined by par 'trackingTargetModule'", path);
        trackingSubjectModule = getModuleByPath(path);
        if (trackingSubjectModule == nullptr)
            throw cRuntimeError("Module not found on path '%s' defined by par 'trackingSubjectModule'", path);
    }
}

void TrackerBase::mapChunks(const Ptr<const Chunk>& chunk, const std::function<void(const Ptr<const Chunk>&, int)>& thunk) const
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

} // namespace tracker

} // namespace inet

