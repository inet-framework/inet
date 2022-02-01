//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/physicallayer/TracingObstacleLossCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(TracingObstacleLossCanvasVisualizer);

TracingObstacleLossCanvasVisualizer::ObstacleLossCanvasVisualization::ObstacleLossCanvasVisualization(LabeledLineFigure *intersectionFigure, cLineFigure *faceNormalFigure1, cLineFigure *faceNormalFigure2) :
    intersectionFigure(intersectionFigure),
    faceNormalFigure1(faceNormalFigure1),
    faceNormalFigure2(faceNormalFigure2)
{
}

TracingObstacleLossCanvasVisualizer::ObstacleLossCanvasVisualization::~ObstacleLossCanvasVisualization()
{
    delete intersectionFigure;
    delete faceNormalFigure1;
    delete faceNormalFigure2;
}

void TracingObstacleLossCanvasVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        cCanvas *canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        obstacleLossLayer = new cGroupFigure("obstacle_loss");
        obstacleLossLayer->setZIndex(zIndex);
        obstacleLossLayer->insertBefore(canvas->getSubmodulesLayer());
    }
}

void TracingObstacleLossCanvasVisualizer::refreshDisplay() const
{
    TracingObstacleLossVisualizerBase::refreshDisplay();
    visualizationTargetModule->getCanvas()->setAnimationSpeed(obstacleLossVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const TracingObstacleLossVisualizerBase::ObstacleLossVisualization *TracingObstacleLossCanvasVisualizer::createObstacleLossVisualization(const ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const
{
    auto object = obstaclePenetratedEvent->object;
    auto intersection1 = obstaclePenetratedEvent->intersection1;
    auto intersection2 = obstaclePenetratedEvent->intersection2;
    auto normal1 = obstaclePenetratedEvent->normal1;
    auto normal2 = obstaclePenetratedEvent->normal2;
    auto loss = obstaclePenetratedEvent->loss;
    const RotationMatrix rotation(object->getOrientation().toEulerAngles());
    const Coord& position = object->getPosition();
    const Coord rotatedIntersection1 = rotation.rotateVector(intersection1);
    const Coord rotatedIntersection2 = rotation.rotateVector(intersection2);
    double intersectionDistance = intersection2.distance(intersection1);
    LabeledLineFigure *intersectionLine = nullptr;
    if (displayIntersections) {
        intersectionLine = new LabeledLineFigure("intersection");
        intersectionLine->setTags((std::string("obstacle_loss ") + tags).c_str());
        intersectionLine->setTooltip("This line represents the intersection of a propagating signal and an obstructing physical object.");
        intersectionLine->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position));
        intersectionLine->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position));
        auto lineFigure = intersectionLine->getLineFigure();
        lineFigure->setLineColor(intersectionLineColor);
        lineFigure->setLineStyle(intersectionLineStyle);
        lineFigure->setLineWidth(intersectionLineWidth);
        lineFigure->setZoomLineWidth(false);
        auto labelFigure = intersectionLine->getLabelFigure();
        char tmp[32];
        sprintf(tmp, "%.4g dB", inet::math::fraction2dB(loss));
        labelFigure->setText(tmp);
    }
    cLineFigure *faceNormal1Line = nullptr;
    cLineFigure *faceNormal2Line = nullptr;
    if (displayFaceNormalVectors) {
        Coord normalVisualization1 = normal1 / normal1.length() * intersectionDistance / 10;
        Coord normalVisualization2 = normal2 / normal2.length() * intersectionDistance / 10;
        faceNormal1Line = new cLineFigure("normal1");
        faceNormal1Line->setTags((std::string("obstacle_loss face_normal_vector ") + tags).c_str());
        faceNormal1Line->setTooltip("This line represents the face normal vector at the intersection of a propagating signal and an obstructing physical object.");
        faceNormal1Line->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position));
        faceNormal1Line->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position + rotation.rotateVector(normalVisualization1)));
        faceNormal1Line->setLineColor(faceNormalLineColor);
        faceNormal1Line->setLineStyle(faceNormalLineStyle);
        faceNormal1Line->setLineWidth(faceNormalLineWidth);
        faceNormal1Line->setZoomLineWidth(false);
        faceNormal2Line = new cLineFigure("normal2");
        faceNormal2Line->setTags((std::string("obstacle_loss face_normal_vector ") + tags).c_str());
        faceNormal2Line->setTooltip("This line represents the face normal vector at the intersection of a propagating signal and an obstructing physical object.");
        faceNormal2Line->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position));
        faceNormal2Line->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position + rotation.rotateVector(normalVisualization2)));
        faceNormal2Line->setLineColor(faceNormalLineColor);
        faceNormal2Line->setLineStyle(faceNormalLineStyle);
        faceNormal2Line->setLineWidth(faceNormalLineWidth);
        faceNormal2Line->setZoomLineWidth(false);
    }
    return new ObstacleLossCanvasVisualization(intersectionLine, faceNormal1Line, faceNormal2Line);
}

void TracingObstacleLossCanvasVisualizer::addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::addObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossCanvasVisualization = static_cast<const ObstacleLossCanvasVisualization *>(obstacleLossVisualization);
    if (displayIntersections)
        obstacleLossLayer->addFigure(obstacleLossCanvasVisualization->intersectionFigure);
    if (displayFaceNormalVectors) {
        obstacleLossLayer->addFigure(obstacleLossCanvasVisualization->faceNormalFigure1);
        obstacleLossLayer->addFigure(obstacleLossCanvasVisualization->faceNormalFigure2);
    }
}

void TracingObstacleLossCanvasVisualizer::removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization)
{
    TracingObstacleLossVisualizerBase::removeObstacleLossVisualization(obstacleLossVisualization);
    auto obstacleLossCanvasVisualization = static_cast<const ObstacleLossCanvasVisualization *>(obstacleLossVisualization);
    if (displayIntersections)
        obstacleLossLayer->removeFigure(obstacleLossCanvasVisualization->intersectionFigure);
    if (displayFaceNormalVectors) {
        obstacleLossLayer->removeFigure(obstacleLossCanvasVisualization->faceNormalFigure1);
        obstacleLossLayer->removeFigure(obstacleLossCanvasVisualization->faceNormalFigure2);
    }
}

void TracingObstacleLossCanvasVisualizer::setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const
{
    auto obstacleLossCanvasVisualization = static_cast<const ObstacleLossCanvasVisualization *>(obstacleLossVisualization);
    if (displayIntersections)
        obstacleLossCanvasVisualization->intersectionFigure->getLineFigure()->setLineOpacity(alpha);
    if (displayFaceNormalVectors) {
        obstacleLossCanvasVisualization->faceNormalFigure1->setLineOpacity(alpha);
        obstacleLossCanvasVisualization->faceNormalFigure2->setLineOpacity(alpha);
    }
}

} // namespace visualizer

} // namespace inet

