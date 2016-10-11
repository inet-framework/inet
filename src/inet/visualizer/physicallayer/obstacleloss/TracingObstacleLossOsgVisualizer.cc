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

#include "inet/common/geometry/common/Rotation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/physicallayer/obstacleloss/TracingObstacleLossOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#endif // ifdef WITH_OSG


namespace inet {

namespace visualizer {

Define_Module(TracingObstacleLossOsgVisualizer);

#ifdef WITH_OSG

void TracingObstacleLossOsgVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        trailNode = new osg::Group();
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
        scene->addChild(trailNode);
    }
}

void TracingObstacleLossOsgVisualizer::obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2)
{
    if (displayIntersectionTrail || displayFaceNormalVectorTrail) {
        const Rotation rotation(object->getOrientation());
        const Coord& position = object->getPosition();
        const Coord rotatedIntersection1 = rotation.rotateVectorClockwise(intersection1);
        const Coord rotatedIntersection2 = rotation.rotateVectorClockwise(intersection2);
        double intersectionDistance = intersection2.distance(intersection1);
        if (displayIntersectionTrail) {
            auto geometry = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection2 + position);
            geometry->setStateSet(inet::osg::createStateSet(cFigure::BLACK, 1.0, false));
            auto geode = new osg::Geode();
            geode->addDrawable(geometry);
            trailNode->addChild(geode);
        }
        if (displayFaceNormalVectorTrail) {
            Coord normalVisualization1 = normal1 / normal1.length() * intersectionDistance / 10;
            Coord normalVisualization2 = normal2 / normal2.length() * intersectionDistance / 10;
            auto geometry1 = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection1 + position + rotation.rotateVectorClockwise(normalVisualization1));
            geometry1->setStateSet(inet::osg::createStateSet(cFigure::RED, 1.0, false));
            auto geometry2 = inet::osg::createLineGeometry(rotatedIntersection2 + position, rotatedIntersection2 + position + rotation.rotateVectorClockwise(normalVisualization2));
            geometry2->setStateSet(inet::osg::createStateSet(cFigure::RED, 1.0, false));
            auto geode = new osg::Geode();
            geode->addDrawable(geometry1);
            geode->addDrawable(geometry2);
            trailNode->addChild(geode);
        }
        if (trailNode->getNumChildren() > 100)
            trailNode->removeChild(0, trailNode->getNumChildren() - 100);
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

