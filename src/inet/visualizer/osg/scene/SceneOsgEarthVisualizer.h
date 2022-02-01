//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENEOSGEARTHVISUALIZER_H
#define __INET_SCENEOSGEARTHVISUALIZER_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/visualizer/osg/base/SceneOsgVisualizerBase.h"

#ifdef WITH_OSGEARTH
#include <osgEarth/GeoTransform>
#include <osgEarth/MapNode>
#include <osgEarthAnnotation/RectangleNode>
#endif // ifdef WITH_OSGEARTH

namespace inet {

namespace visualizer {

class INET_API SceneOsgEarthVisualizer : public SceneOsgVisualizerBase
{
#ifdef WITH_OSGEARTH

  public:
    osgEarth::MapNode *getMapNode() { return mapNode; }

  protected:
    ModuleRefByPar<IGeographicCoordinateSystem> coordinateSystem;
    double cameraDistanceFactor = NaN;

    osgEarth::MapNode *mapNode = nullptr;
    osgEarth::GeoTransform *geoTransform = nullptr;
    osg::PositionAttitudeTransform *localTransform = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void initializeScene() override;
    virtual void initializeLocator();
    virtual void initializeViewpoint();

#endif // ifdef WITH_OSGEARTH
};

} // namespace visualizer

} // namespace inet

#endif

