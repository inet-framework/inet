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

#ifndef __INET_SCENEOSGEARTHVISUALIZER_H
#define __INET_SCENEOSGEARTHVISUALIZER_H

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/visualizer/base/SceneOsgVisualizerBase.h"

#ifdef WITH_OSGEARTH
#include <osgEarth/MapNode>
#include <osgEarthAnnotation/RectangleNode>
#include <osgEarth/GeoTransform>
#endif // ifdef WITH_OSGEARTH

namespace inet {

namespace visualizer {

class INET_API SceneOsgEarthVisualizer : public SceneOsgVisualizerBase
{
#ifdef WITH_OSGEARTH

  public:
    osgEarth::MapNode *getMapNode() { return mapNode; }

  protected:
    IGeographicCoordinateSystem *coordinateSystem = nullptr;
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

#endif // ifndef __INET_SCENEOSGEARTHVISUALIZER_H

