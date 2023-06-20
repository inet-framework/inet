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

#ifndef __INET_TRACKERBASE_H
#define __INET_TRACKERBASE_H

#include <functional>
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/chunk/Chunk.h"

namespace inet {

namespace tracker {

class INET_API TrackerBase : public cSimpleModule
{
  protected:
    cModule *trackingTargetModule = nullptr;
    cModule *trackingSubjectModule = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void mapChunks(const Ptr<const Chunk>& chunk, const std::function<void(const Ptr<const Chunk>&, int)>& thunk) const;
};

} // namespace tracker

} // namespace inet

#endif

