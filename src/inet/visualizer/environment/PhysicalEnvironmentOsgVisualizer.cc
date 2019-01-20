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

#include "inet/common/ModuleAccess.h"
#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/common/geometry/shape/Prism.h"
#include "inet/common/geometry/shape/Sphere.h"
#include "inet/visualizer/environment/PhysicalEnvironmentOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Material>
#include <osg/PolygonMode>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Texture2D>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

Define_Module(PhysicalEnvironmentOsgVisualizer);

#ifdef WITH_OSG

void PhysicalEnvironmentOsgVisualizer::initialize(int stage)
{
    PhysicalEnvironmentVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        enableObjectOpacity = par("enableObjectOpacity");
}

void PhysicalEnvironmentOsgVisualizer::refreshDisplay() const
{
    // only update after initialize
    if (physicalEnvironment != nullptr && getSimulation()->getEventNumber() == 0 && displayObjects) {
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        for (int i = 0; i < physicalEnvironment->getNumObjects(); i++) {
            const IPhysicalObject *object = physicalEnvironment->getObject(i);
            const ShapeBase *shape = object->getShape();
            const Coord& position = object->getPosition();
            const Quaternion& orientation = object->getOrientation();
            const RotationMatrix rotation(orientation.toEulerAngles());
            const double opacity = enableObjectOpacity? object->getOpacity() : 1.0;
            // cuboid
            const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
            if (cuboid != nullptr) {
                auto size = cuboid->getSize();
                auto drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3d(0, 0, 0), size.x, size.y, size.z));
                auto stateSet = inet::osg::createStateSet(object->getFillColor(), opacity);
                drawable->setStateSet(stateSet);
                auto geode = new osg::Geode();
                geode->addDrawable(drawable);
                auto pat = inet::osg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(geode);
                scene->addChild(pat);
            }
            // sphere
            const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
            if (sphere != nullptr) {
                double radius = sphere->getRadius();
                auto drawable = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(position.x, position.y, position.z), radius));
                drawable->setStateSet(inet::osg::createStateSet(object->getFillColor(), opacity));
                auto geode = new osg::Geode();
                geode->addDrawable(drawable);
                scene->addChild(geode);
            }
            // prism
            const Prism *prism = dynamic_cast<const Prism *>(shape);
            if (prism != nullptr) {
                auto geode = new osg::Geode();
                auto stateSet = inet::osg::createStateSet(object->getFillColor(), opacity);
                auto bottomFace = inet::osg::createPolygonGeometry(prism->getBase().getPoints());
                bottomFace->setStateSet(stateSet);
                auto topFace = inet::osg::createPolygonGeometry(prism->getBase().getPoints(), Coord(0, 0, prism->getHeight()));
                topFace->setStateSet(stateSet);
                geode->addDrawable(bottomFace);
                geode->addDrawable(topFace);
                for (auto face : prism->getFaces()) {
                    auto sideFace = inet::osg::createPolygonGeometry(face.getPoints());
                    sideFace->setStateSet(stateSet);
                    geode->addDrawable(sideFace);
                }
                auto pat = inet::osg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(geode);
                scene->addChild(pat);
            }
            // polyhedron
            const Polyhedron *polyhedron = dynamic_cast<const Polyhedron *>(shape);
            if (polyhedron != nullptr) {
                auto geode = new osg::Geode();
                auto stateSet = inet::osg::createStateSet(object->getFillColor(), opacity);
                for (auto face : polyhedron->getFaces()) {
                    auto geometry = new osg::Geometry();
                    auto vertices = new osg::Vec3Array();
                    for (auto edge : face->getEdges())
                        vertices->push_back(osg::Vec3d(edge->getP1()->x, edge->getP1()->y, edge->getP1()->z));
                    geometry->setVertexArray(vertices);
                    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, face->getEdges().size()));
                    geometry->setStateSet(stateSet);
                    geode->addDrawable(geometry);
                }
                auto pat = inet::osg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(geode);
                scene->addChild(pat);
            }
        }
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

