//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OPENSTREETMAPSCENECANVASVISUALIZER_H
#define __INET_OPENSTREETMAPSCENECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/streetmap/OpenStreetMap.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API OpenStreetMapSceneCanvasVisualizer : public SceneVisualizerBase
{
  protected:
    ModuleRefByPar<IGeographicCoordinateSystem> coordinateSystem;
    CanvasProjection *canvasProjection = nullptr;

  protected:
    virtual void initialize(int stage) override;
    cFigure::Point toCanvas(const osm::OpenStreetMap& map, double lat, double lon);
    cGroupFigure *createMapFigure(const osm::OpenStreetMap& map);
};

} // namespace visualizer

} // namespace inet

#endif

