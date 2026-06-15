//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/environment/PhysicalEnvironmentVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/shape/Prism.h"
#include "inet/common/geometry/shape/Sphere.h"
#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

Define_Module(PhysicalEnvironmentVsgVisualizer);

void PhysicalEnvironmentVsgVisualizer::initialize(int stage)
{
    PhysicalEnvironmentVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        enableObjectOpacity = par("enableObjectOpacity");
}

void PhysicalEnvironmentVsgVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    // only populate geometry once, at event 0 (static environment)
    if (physicalEnvironment != nullptr && getSimulation()->getEventNumber() == 0 && displayObjects) {
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        for (int i = 0; i < physicalEnvironment->getNumObjects(); i++) {
            const IPhysicalObject *object = physicalEnvironment->getObject(i);
            const ShapeBase *shape = object->getShape();
            const Coord& position = object->getPosition();
            const Quaternion& orientation = object->getOrientation();
            const RotationMatrix rotation(orientation.toEulerAngles());
            const double opacity = enableObjectOpacity ? object->getOpacity() : 1.0;
            const cFigure::Color& fillColor = object->getFillColor();

            // cuboid
            const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
            if (cuboid != nullptr) {
                auto size = cuboid->getSize();
                auto box = inet::vsg::createBox(Coord::ZERO, size, fillColor, opacity);
                auto pat = inet::vsg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(box);
                scene->addChild(pat);
            }

            // sphere
            const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
            if (sphere != nullptr) {
                auto node = inet::vsg::createSphere(position, sphere->getRadius(), fillColor, opacity);
                scene->addChild(node);
            }

            // prism: render base polygon (bottom), translated top polygon, and each side face as a polygon
            const Prism *prism = dynamic_cast<const Prism *>(shape);
            if (prism != nullptr) {
                auto group = ::vsg::Group::create();
                const auto& basePoints = prism->getBase().getPoints();
                // bottom face (translation = Coord::ZERO)
                group->addChild(inet::vsg::createPolygon(basePoints, fillColor, opacity));
                // top face (translated by height along local Z)
                Coord topOffset(0, 0, prism->getHeight());
                group->addChild(inet::vsg::createPolygon(basePoints, fillColor, opacity, topOffset));
                // side faces
                for (const auto& face : prism->getFaces())
                    group->addChild(inet::vsg::createPolygon(face.getPoints(), fillColor, opacity));
                auto pat = inet::vsg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(group);
                scene->addChild(pat);
            }

            // polyhedron: render each face as a polygon
            const Polyhedron *polyhedron = dynamic_cast<const Polyhedron *>(shape);
            if (polyhedron != nullptr) {
                auto group = ::vsg::Group::create();
                for (const auto *face : polyhedron->getFaces()) {
                    std::vector<Coord> pts;
                    for (const auto *edge : face->getEdges())
                        pts.emplace_back(edge->getP1()->x, edge->getP1()->y, edge->getP1()->z);
                    if (!pts.empty())
                        group->addChild(inet::vsg::createPolygon(pts, fillColor, opacity));
                }
                auto pat = inet::vsg::createPositionAttitudeTransform(position, orientation);
                pat->addChild(group);
                scene->addChild(pat);
            }

            // TODO: other ShapeBase subclasses (e.g. custom shapes) are not handled; bounding box approximation would require computeBoundingBoxSize()
        }
    }
}

} // namespace visualizer

} // namespace inet
