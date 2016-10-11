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

#ifndef __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H
#define __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H

#include "inet/common/figures/TrailFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

class INET_API TracingObstacleLossCanvasVisualizer : public TracingObstacleLossVisualizerBase
{
  protected:
    double zIndex = NaN;
    /** @name Graphics */
    //@{
    /**
     * The 2D projection used on the canvas.
     */
    const CanvasProjection *canvasProjection = nullptr;
    /**
     * The trail figures that represent the last couple of obstacle intersections.
     */
    TrailFigure *intersectionTrail = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H

