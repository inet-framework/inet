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

#ifndef __INET_OPENSTREETMAPSCENECANVASVISUALIZER_H
#define __INET_OPENSTREETMAPSCENECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/streetmap/OpenStreetMap.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API OpenStreetMapSceneCanvasVisualizer : public SceneVisualizerBase
{
  protected:
    IGeographicCoordinateSystem *coordinateSystem = nullptr;
    CanvasProjection *canvasProjection = nullptr;

  protected:
    virtual void initialize(int stage) override;
    cFigure::Point toCanvas(const osm::OpenStreetMap& map, double lat, double lon);
    cGroupFigure *createMapFigure(const osm::OpenStreetMap& map);
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_OPENSTREETMAPSCENECANVASVISUALIZER_H

