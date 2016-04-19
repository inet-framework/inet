//
// Copyright (C) 2013 OpenSim Ltd.
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
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/scene/SceneOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Group>
#include <osgDB/ReadFile>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(SceneOsgVisualizer);

#ifdef WITH_OSG

void SceneOsgVisualizer::initialize(int stage)
{
    SceneOsgVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        initializeScene();
    else if (stage == INITSTAGE_LAST) {
        if (par("displayPlayground"))
            initializePlayground();
        double axisLength = par("axisLength");
        if (!std::isnan(axisLength))
            initializeAxis(axisLength);
        initializeViewpoint();
    }
}

void SceneOsgVisualizer::initializeViewpoint()
{
    auto boundingSphere = getNetworkBoundingSphere();
    auto center = boundingSphere.center();
    auto radius = boundingSphere.radius();
    double cameraDistanceFactor = par("cameraDistanceFactor");
    auto eye = cOsgCanvas::Vec3d(center.x() + cameraDistanceFactor * radius, center.y() + cameraDistanceFactor * radius, center.z() + cameraDistanceFactor * radius);
    auto viewpointCenter = cOsgCanvas::Vec3d(center.x(), center.y(), center.z());
    auto osgCanvas = visualizerTargetModule->getOsgCanvas();
    osgCanvas->setGenericViewpoint(cOsgCanvas::Viewpoint(eye, viewpointCenter, cOsgCanvas::Vec3d(0, 0, 1)));
}

osg::Group *SceneOsgVisualizer::getMainPart()
{
    auto osgCanvas = visualizerTargetModule->getOsgCanvas();
    return check_and_cast<osg::Group *>(osgCanvas->getScene());
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

