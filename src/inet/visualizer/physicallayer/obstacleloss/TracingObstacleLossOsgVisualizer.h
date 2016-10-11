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

#ifndef __INET_TRACINGOBSTACLELOSSOSGVISUALIZER_H
#define __INET_TRACINGOBSTACLELOSSOSGVISUALIZER_H

#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

class INET_API TracingObstacleLossOsgVisualizer : public TracingObstacleLossVisualizerBase
{
#ifdef WITH_OSG

  protected:
    osg::Group *trailNode = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) override;

#else // ifdef WITH_OSG

  public:
    virtual void obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) override {}

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_TRACINGOBSTACLELOSSOSGVISUALIZER_H

