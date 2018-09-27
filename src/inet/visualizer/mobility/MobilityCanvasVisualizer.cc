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
#include "inet/visualizer/mobility/MobilityCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(MobilityCanvasVisualizer);

MobilityCanvasVisualizer::MobilityCanvasVisualization::MobilityCanvasVisualization(cOvalFigure *positionFigure, cPieSliceFigure *orientationFigure, cLineFigure *veloctiyFigure, TrailFigure *trailFigure, IMobility *mobility) :
    MobilityVisualization(mobility),
    positionFigure(positionFigure),
    orientationFigure(orientationFigure),
    veloctiyFigure(veloctiyFigure),
    trailFigure(trailFigure)
{
}

MobilityCanvasVisualizer::~MobilityCanvasVisualizer()
{
    for (auto mobilityVisualization : mobilityVisualizations)
        delete mobilityVisualization.second;
}

void MobilityCanvasVisualizer::initialize(int stage)
{
    MobilityVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
    }
}

void MobilityCanvasVisualizer::refreshDisplay() const
{
    MobilityVisualizerBase::refreshDisplay();
    for (auto it : mobilityVisualizations) {
        auto mobilityVisualization = it.second;
        auto mobility = mobilityVisualization->mobility;
        auto position = canvasProjection->computeCanvasPoint(mobility->getCurrentPosition());
        auto orientation = mobility->getCurrentAngularPosition();
        auto velocity = canvasProjection->computeCanvasPoint(mobility->getCurrentVelocity());
        if (displayPositions) {
            double radius = positionCircleRadius / getEnvir()->getZoomLevel(visualizationTargetModule);
            mobilityVisualization->positionFigure->setBounds(cFigure::Rectangle(position.x - radius, position.y - radius, 2 * radius, 2 * radius));
        }
        if (displayOrientations) {
            // NOTE: this negation cancels out the (incorrect) CCW angle handling of cPieSliceFigure (see bug https://dev.omnetpp.org/bugs/view.php?id=1030)
            auto angle = -orientation.toEulerAngles().alpha;
            mobilityVisualization->orientationFigure->setStartAngle(rad(angle - rad(M_PI) * orientationPieSize).get());
            mobilityVisualization->orientationFigure->setEndAngle(rad(angle + rad(M_PI) * orientationPieSize).get());
            double radius = orientationPieRadius / getEnvir()->getZoomLevel(visualizationTargetModule);
            mobilityVisualization->orientationFigure->setBounds(cFigure::Rectangle(position.x - radius, position.y - radius, 2 * radius, 2 * radius));
        }
        if (displayVelocities) {
            mobilityVisualization->veloctiyFigure->setStart(position);
            mobilityVisualization->veloctiyFigure->setEnd(position + velocity * velocityArrowScale);
            mobilityVisualization->veloctiyFigure->setVisible(velocity.getLength() != 0);
        }
        if (displayMovementTrails)
            extendMovementTrail(mobility, mobilityVisualization->trailFigure, position);
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(mobilityVisualizations.empty() ? 0 : animationSpeed, this);
}

MobilityCanvasVisualizer::MobilityCanvasVisualization *MobilityCanvasVisualizer::getMobilityVisualization(const IMobility *mobility) const
{
    auto it = mobilityVisualizations.find(mobility);
    if (it == mobilityVisualizations.end())
        return nullptr;
    else
        return it->second;
}

void MobilityCanvasVisualizer::setMobilityVisualization(const IMobility *mobility, MobilityCanvasVisualization *entry)
{
    mobilityVisualizations[mobility] = entry;
}

void MobilityCanvasVisualizer::removeMobilityVisualization(const IMobility *mobility)
{
    mobilityVisualizations.erase(mobility);
}

MobilityCanvasVisualizer::MobilityCanvasVisualization* MobilityCanvasVisualizer::ensureMobilityVisualization(IMobility *mobility)
{
    auto mobilityVisualization = getMobilityVisualization(mobility);
    if (mobilityVisualization == nullptr) {
        auto canvas = visualizationTargetModule->getCanvas();
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
        cOvalFigure *positionFigure = nullptr;
        if (displayPositions) {
            positionFigure = new cOvalFigure("position");
            positionFigure->setTags((std::string("position ") + tags).c_str());
            positionFigure->setTooltip("This circle represents the current position of the mobility model");
            positionFigure->setZIndex(zIndex);
            positionFigure->setLineColor(positionCircleLineColorSet.getColor(module->getId()));
            positionFigure->setLineWidth(positionCircleLineWidth);
            positionFigure->setFilled(true);
            positionFigure->setFillColor(positionCircleFillColorSet.getColor(module->getId()));
            canvas->addFigure(positionFigure);
        }
        cPieSliceFigure *orientationFigure = nullptr;
        if (displayOrientations) {
            orientationFigure = new cPieSliceFigure("orientation");
            orientationFigure->setTags((std::string("orientation ") + tags).c_str());
            orientationFigure->setTooltip("This arc represents the current orientation of the mobility model");
            orientationFigure->setZIndex(zIndex);
            orientationFigure->setLineOpacity(orientationPieOpacity);
            orientationFigure->setLineColor(orientationLineColor);
            orientationFigure->setLineStyle(orientationLineStyle);
            orientationFigure->setLineWidth(orientationLineWidth);
            orientationFigure->setFilled(true);
            orientationFigure->setFillOpacity(orientationPieOpacity);
            orientationFigure->setFillColor(orientationFillColor);
            canvas->addFigure(orientationFigure);
        }
        cLineFigure *velocityFigure = nullptr;
        if (displayVelocities) {
            velocityFigure = new cLineFigure("velocity");
            velocityFigure->setTags((std::string("velocity ") + tags).c_str());
            velocityFigure->setTooltip("This arrow represents the current velocity of the mobility model");
            velocityFigure->setZIndex(zIndex);
            velocityFigure->setVisible(false);
            velocityFigure->setEndArrowhead(cFigure::ARROW_SIMPLE);
            velocityFigure->setLineColor(velocityLineColor);
            velocityFigure->setLineStyle(velocityLineStyle);
            velocityFigure->setLineWidth(velocityLineWidth);
            canvas->addFigure(velocityFigure);
        }
        TrailFigure *trailFigure = nullptr;
        if (displayMovementTrails) {
            trailFigure = new TrailFigure(trailLength, true, "movement trail");
            trailFigure->setTags((std::string("movement_trail recent_history ") + tags).c_str());
            trailFigure->setZIndex(zIndex);
            canvas->addFigure(trailFigure);
        }
        mobilityVisualization = new MobilityCanvasVisualization(positionFigure, orientationFigure, velocityFigure, trailFigure, mobility);
        setMobilityVisualization(mobility, mobilityVisualization);
    }
    return mobilityVisualization;
}

void MobilityCanvasVisualizer::extendMovementTrail(const IMobility *mobility, TrailFigure *trailFigure, cFigure::Point position) const
{
    cFigure::Point startPosition;
    cFigure::Point endPosition = position;
    if (trailFigure->getNumFigures() == 0)
        startPosition = position;
    else
        startPosition = static_cast<cLineFigure*>(trailFigure->getFigure(trailFigure->getNumFigures() - 1))->getEnd();
    double dx = startPosition.x - endPosition.x;
    double dy = startPosition.y - endPosition.y;
    // TODO: 1?
    if (trailFigure->getNumFigures() == 0 || dx * dx + dy * dy > 1) {
        cLineFigure *movementLine = new cLineFigure("movementTrail");
        movementLine->setTags((std::string("movement_trail recent_history ") + tags).c_str());
        movementLine->setTooltip("This line represents the recent movement trail of the mobility model");
        movementLine->setStart(startPosition);
        movementLine->setEnd(endPosition);
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
        movementLine->setLineColor(movementTrailLineColorSet.getColor(module->getId()));
        movementLine->setLineStyle(movementTrailLineStyle);
        movementLine->setLineWidth(movementTrailLineWidth);
        movementLine->setZoomLineWidth(false);
        trailFigure->addFigure(movementLine);
    }
}

void MobilityCanvasVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule *>(source)))
            ensureMobilityVisualization(dynamic_cast<IMobility *>(source));
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (dynamic_cast<cPreModuleDeleteNotification *>(object)) {
            if (auto mobility = dynamic_cast<IMobility *>(source))
                removeMobilityVisualization(mobility);
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

