//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/physicallayer/TracingObstacleLossOsgVisualizer.h"

#include <osg/Geode>
#include <osg/LineWidth>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(TracingObstacleLossOsgVisualizer);

void TracingObstacleLossOsgVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        obstacleLossNode = new osg::Group();
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(obstacleLossNode);
    }
}

void TracingObstacleLossOsgVisualizer::refreshDisplay() const
{
    TracingObstacleLossVisualizerBase::refreshDisplay();
    // TODO switch to osg canvas when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(obstacleLossVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const TracingObstacleLossVisualizerBase::ObstacleLossVisualization *TracingObstacleLossOsgVisualizer::createObstacleLossVisualization(const ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const
{
    auto object = obstaclePenetratedEvent->object;
    auto intersection1 = obstaclePenetratedEvent->intersection1;
    auto intersection2 = obstaclePenetratedEvent->intersection2;
    auto normal1 = obstaclePenetratedEvent->normal1;
    auto normal2 = obstaclePenetratedEvent->normal2;
    // TODO display auto loss = obstaclePenetratedEvent->loss;
    const RotationMatrix rotation(object->getOrientation().toEulerAngles());
    const Coord& position = object->getPosition();
    const Coord rotatedIntersection1 = rotation.rotateVector(intersection1);
    const Coord rotatedIntersection2 = rotation.rotateVector(intersection2);
    double intersectionDistance = intersection2.distance(intersection1);
    auto group = new osg::Group();
    if (displayIntersections) {
        auto geometry = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection2 + position);
        auto geode = new osg::Geode();
        geode->addDrawable(geometry);
        geode->setStateSet(inet::osg::createLineStateSet(intersectionLineColor, intersectionLineStyle, intersectionLineWidth));
        group->addChild(geode);
    }
    if (displayFaceNormalVectors) {
        Coord normalVisualization1 = normal1 / normal1.length() * intersectionDistance / 10;
        Coord normalVisualization2 = normal2 / normal2.length() * intersectionDistance / 10;
        auto geometry1 = inet::osg::createLineGeometry(rotatedIntersection1 + position, rotatedIntersection1 + position + rotation.rotateVector(normalVisualization1));
        auto geometry2 = inet::osg::createLineGeometry(rotatedIntersection2 + position, rotatedIntersection2 + position + rotation.rotateVector(normalVisualization2));
        auto geode = new osg::Geode();
        geode->addDrawable(geometry1);
        geode->addDrawable(geometry2);
        geode->setStateSet(inet::osg::createLineStateSet(faceNormalLineColor, faceNormalLineStyle, faceNormalLineWidth));
        group->addChild(geode);
    }
    return new ObstacleLossOsgVisualization(group);
}

void TracingObstacleLossOsgVisualizer::addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::addObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(obstacleLossOsgVisualization->node);
}

void TracingObstacleLossOsgVisualizer::removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::removeObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto node = obstacleLossOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void TracingObstacleLossOsgVisualizer::setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const
{
    auto obstacleLossOsgVisualization = static_cast<const ObstacleLossOsgVisualization *>(obstacleLossVisualization);
    auto node = obstacleLossOsgVisualization->node;
    for (unsigned int i = 0; i < node->getNumChildren(); i++) {
        auto material = static_cast<osg::Material *>(node->getChild(i)->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
        material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
    }
}

} // namespace visualizer

} // namespace inet

