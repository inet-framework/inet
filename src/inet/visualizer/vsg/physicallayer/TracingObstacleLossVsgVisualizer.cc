//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/physicallayer/TracingObstacleLossVsgVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(TracingObstacleLossVsgVisualizer);

void TracingObstacleLossVsgVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        obstacleLossNode = ::vsg::Group::create();
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(obstacleLossNode);
    }
}

void TracingObstacleLossVsgVisualizer::refreshDisplay() const
{
    TracingObstacleLossVisualizerBase::refreshDisplay();
    // Drive the 2D canvas animation speed for fade-out timing (same as the OSG path).
    visualizationTargetModule->getCanvas()->setAnimationSpeed(obstacleLossVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const TracingObstacleLossVisualizerBase::ObstacleLossVisualization *
TracingObstacleLossVsgVisualizer::createObstacleLossVisualization(const ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const
{
    auto object        = obstaclePenetratedEvent->object;
    auto intersection1 = obstaclePenetratedEvent->intersection1;
    auto intersection2 = obstaclePenetratedEvent->intersection2;
    auto normal1       = obstaclePenetratedEvent->normal1;
    auto normal2       = obstaclePenetratedEvent->normal2;
    // TODO: display obstaclePenetratedEvent->loss as a label or color

    // A nullptr object (e.g. terrain from TerrainObstacleLoss) means the intersection
    // points and normals are already world-frame: identity rotation, no offset.
    const RotationMatrix rotation = object != nullptr ? RotationMatrix(object->getOrientation().toEulerAngles()) : RotationMatrix();
    const Coord position = object != nullptr ? object->getPosition() : Coord::ZERO;
    const Coord rotatedIntersection1 = rotation.rotateVector(intersection1) + position;
    const Coord rotatedIntersection2 = rotation.rotateVector(intersection2) + position;

    auto group = ::vsg::Group::create();

    if (displayIntersections) {
        // Line segment along the signal path through the obstacle
        auto line = inet::vsg::createLine(rotatedIntersection1, rotatedIntersection2,
                cFigure::ARROW_NONE, cFigure::ARROW_NONE,
                intersectionLineColor, intersectionLineStyle, intersectionLineWidth);
        group->addChild(line);
    }

    if (displayFaceNormalVectors) {
        double intersectionDistance = intersection2.distance(intersection1);
        // Scale normals to 1/10 of the intersection segment length (same as OSG)
        Coord n1scaled = normal1 / normal1.length() * intersectionDistance / 10;
        Coord n2scaled = normal2 / normal2.length() * intersectionDistance / 10;
        Coord normalEnd1 = rotatedIntersection1 + rotation.rotateVector(n1scaled);
        Coord normalEnd2 = rotatedIntersection2 + rotation.rotateVector(n2scaled);
        auto normalLine1 = inet::vsg::createLine(rotatedIntersection1, normalEnd1,
                cFigure::ARROW_NONE, cFigure::ARROW_NONE,
                faceNormalLineColor, faceNormalLineStyle, faceNormalLineWidth);
        auto normalLine2 = inet::vsg::createLine(rotatedIntersection2, normalEnd2,
                cFigure::ARROW_NONE, cFigure::ARROW_NONE,
                faceNormalLineColor, faceNormalLineStyle, faceNormalLineWidth);
        group->addChild(normalLine1);
        group->addChild(normalLine2);
    }

    return new ObstacleLossVsgVisualization(group);
}

void TracingObstacleLossVsgVisualizer::addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::addObstacleLossVisualization(obstacleLossVisualization);
    auto vsgVis = static_cast<const ObstacleLossVsgVisualization *>(obstacleLossVisualization);
    obstacleLossNode->addChild(vsgVis->node);
}

void TracingObstacleLossVsgVisualizer::removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::removeObstacleLossVisualization(obstacleLossVisualization);
    auto vsgVis = static_cast<const ObstacleLossVsgVisualization *>(obstacleLossVisualization);
    auto& ch = obstacleLossNode->children;
    ::vsg::ref_ptr<::vsg::Node> nodeRef(vsgVis->node);
    ch.erase(std::remove(ch.begin(), ch.end(), nodeRef), ch.end());
}

void TracingObstacleLossVsgVisualizer::setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const
{
    // TODO: VSG bakes opacity into geometry at construction time (no detachable
    //       StateSet/Material).  To implement fade-out, the child nodes would need
    //       to be rebuilt with the new opacity value.  For now, the visualization
    //       appears at full opacity until it is removed by the base-class fade timer.
    (void)obstacleLossVisualization;
    (void)alpha;
}

} // namespace visualizer

} // namespace inet
