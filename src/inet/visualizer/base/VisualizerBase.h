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

#ifndef __INET_VISUALIZERBASE_H
#define __INET_VISUALIZERBASE_H

#include <functional>

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

namespace visualizer {

class INET_API VisualizerBase : public cSimpleModule
{
  protected:
    cModule *visualizationTargetModule = nullptr;
    cModule *visualizationSubjectModule = nullptr;
    const char *tags = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual Coord getPosition(const cModule *networkNode) const;
    virtual Coord getContactPosition(const cModule *networkNode, const Coord& fromPosition, const char *contactMode, double contactSpacing) const;
    virtual Quaternion getOrientation(const cModule *networkNode) const;

    virtual void mapChunkIds(const Ptr<const Chunk>& chunk, const std::function<void(int)>& thunk) const;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_VISUALIZERBASE_H

