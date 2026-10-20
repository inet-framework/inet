//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/mobility/MobilityCanvasVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(MobilityCanvasVisualizer);

static CanvasProjection::DirectionProjection parseDirectionProjection(const char *value)
{
    if (!strcmp(value, "projected"))
        return CanvasProjection::DIRECTION_PROJECTED;
    else if (!strcmp(value, "trueLength"))
        return CanvasProjection::DIRECTION_TRUE_LENGTH;
    else if (!strcmp(value, "isometric"))
        return CanvasProjection::DIRECTION_ISOMETRIC;
    else
        throw cRuntimeError("Unknown direction projection: '%s'", value);
}

MobilityCanvasVisualizer::MobilityCanvasVisualization::~MobilityCanvasVisualization()
{
    delete positionFigure;
    delete orientationFigure;
    delete velocityFigure;
    delete trailFigure;
}

MobilityCanvasVisualizer::MobilityCanvasVisualization::MobilityCanvasVisualization(cOvalFigure *positionFigure, cGroupFigure *orientationFigure, cLineFigure *velocityFigure, TrailFigure *trailFigure, IMobility *mobility) :
    MobilityVisualization(mobility),
    positionFigure(positionFigure),
    orientationFigure(orientationFigure),
    velocityFigure(velocityFigure),
    trailFigure(trailFigure)
{
}

void MobilityCanvasVisualizer::initialize(int stage)
{
    MobilityVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
        mobilityGroup = new ProjectionClipFigure("mobility", canvasProjection);
        mobilityGroup->setZIndex(zIndex);
        visualizationTargetModule->getCanvas()->addFigure(mobilityGroup);
        velocityProjection = parseDirectionProjection(par("velocityProjection"));
        orientationProjection = parseDirectionProjection(par("orientationProjection"));
    }
}

void MobilityCanvasVisualizer::refreshDisplay() const
{
    MobilityVisualizerBase::refreshDisplay();
    for (auto it : mobilityVisualizations) {
        auto mobilityVisualization = static_cast<MobilityCanvasVisualization *>(it.second);
        auto mobility = mobilityVisualization->mobility;
        auto positionCoord = mobility->getCurrentPosition();
        auto position = canvasProjection->computeCanvasPoint(positionCoord);
        auto orientation = mobility->getCurrentAngularPosition();
        // The velocity arrow and the orientation cone are direction (not position) indicators; they are
        // projected with CanvasProjection::computeCanvasDirection() using the per-indicator velocityProjection
        // and orientationProjection modes, which select how (if at all) they follow the equirectangular map
        // distortion (see CanvasProjection::DirectionProjection).
        if (displayPositions) {
            double radius = positionCircleRadius / getEnvir()->getZoomLevel(visualizationTargetModule);
            cFigure::Rectangle bounds(position.x - radius, position.y - radius, 2 * radius, 2 * radius);
            mobilityVisualization->positionFigure->setBounds(bounds);
        }
        if (displayOrientations) {
            // Draw the orientation as a 3D cone whose axis is the node's forward direction, rendered as
            // its (rotated) base ellipse plus the two cone-side lines tangent from the apex: a triangle
            // when the axis lies in the view plane, the base ellipse when it points out of plane. The
            // direction projection above preserves 3D angles, so the velocity stays perpendicular to the disc.
            auto axis = orientation.rotate(Coord::X_AXIS); // unit forward direction in the scene frame
            double coneRadius = std::tan(M_PI * orientationPieSize); // base radius of a unit-length cone
            // orthonormal basis of the cone's base plane (perpendicular to the axis), in scene space
            Coord ref = std::fabs(axis.x) < 0.9 ? Coord::X_AXIS : Coord::Y_AXIS;
            Coord u = axis % ref; u.normalize();
            Coord v = axis % u; v.normalize();
            // apex-relative reference vectors of the unit-length cone;
            // the axis projection also yields the out-of-plane component used for the toward/away cue
            double axisDepth;
            cFigure::Point axisRef = canvasProjection->computeCanvasDirection(positionCoord, axis, axisDepth, orientationProjection);
            cFigure::Point uRef = canvasProjection->computeCanvasDirection(positionCoord, u * coneRadius, orientationProjection);
            cFigure::Point vRef = canvasProjection->computeCanvasDirection(positionCoord, v * coneRadius, orientationProjection);
            bool toward = axisDepth >= 0; // the axis points up / toward the viewer
            // scale the reference figure up so its largest extent matches the requested pixel size
            double zoom = getEnvir()->getZoomLevel(visualizationTargetModule);
            double refExtent = std::max(std::max(axisRef.getLength(), uRef.getLength()), vRef.getLength());
            auto baseFigure = mobilityVisualization->orientationBaseFigure;
            bool valid = refExtent >= 1e-9; // a degenerate projection produces nothing to draw
            baseFigure->setVisible(valid);
            if (valid) {
                double scale = (orientationPieRadius / zoom) / refExtent;
                cFigure::Point C = position + axisRef * scale; // projected base center
                cFigure::Point A = uRef * scale;               // conjugate semi-diameter
                cFigure::Point B = vRef * scale;               // conjugate semi-diameter
                // base ellipse: recover its axis-aligned semi-axes and angle from A, B (eigen of M*M^T)
                double m00 = A.x * A.x + B.x * B.x, m01 = A.x * A.y + B.x * B.y, m11 = A.y * A.y + B.y * B.y;
                double mid = (m00 + m11) / 2, dif = std::sqrt(std::max(0.0, (m00 - m11) * (m00 - m11) / 4 + m01 * m01));
                double semiMajor = std::sqrt(std::max(0.0, mid + dif)), semiMinor = std::sqrt(std::max(0.0, mid - dif));
                double angle = 0.5 * std::atan2(2 * m01, m00 - m11);
                baseFigure->setBounds(cFigure::Rectangle(-semiMajor, -semiMinor, 2 * semiMajor, 2 * semiMinor));
                baseFigure->setTransform(cFigure::Transform(std::cos(angle), std::sin(angle), -std::sin(angle), std::cos(angle), C.x, C.y));
                // fill the base disc with the up/down color (if specified); otherwise leave it unfilled
                bool fill = toward ? orientationFillUp : orientationFillDown;
                if (fill)
                    baseFigure->setFillColor(toward ? orientationFillColorUp : orientationFillColorDown);
                baseFigure->setFilled(fill);
                // cone-side lines: the two tangents from the apex to the ellipse q(t)=C+cos(t)A+sin(t)B,
                // found from (q(t)-position) x q'(t) = 0  ->  p*cos(t) + w*sin(t) = -(A x B)
                cFigure::Point D = C - position;
                double crossAB = A.x * B.y - A.y * B.x;
                double p = D.x * B.y - D.y * B.x, w = -(D.x * A.y - D.y * A.x);
                double rr = std::sqrt(p * p + w * w);
                bool apexOutside = rr > 1e-9 && std::fabs(crossAB) < rr; // tangents exist only outside the ellipse
                for (int s = 0; s < 2; s++) {
                    auto line = mobilityVisualization->orientationLineFigure[s];
                    if (apexOutside) {
                        double t = std::atan2(w, p) + (s == 0 ? 1 : -1) * std::acos(-crossAB / rr);
                        cFigure::Point tangent = C + A * std::cos(t) + B * std::sin(t);
                        line->setStart(position);
                        line->setEnd(tangent);
                    }
                    line->setVisible(apexOutside); // hidden when the apex falls inside the base ellipse (face-on)
                }
            }
            else {
                mobilityVisualization->orientationLineFigure[0]->setVisible(false);
                mobilityVisualization->orientationLineFigure[1]->setVisible(false);
            }
        }
        if (displayVelocities) {
            auto velocityCoord = mobility->getCurrentVelocity();
            // project the velocity as a direction at the node using the selected velocityProjection mode
            // (with "isometric" it stays perpendicular to the base disc of an "isometric" orientation cone)
            auto velocity = canvasProjection->computeCanvasDirection(positionCoord, velocityCoord, velocityProjection);
            mobilityVisualization->velocityFigure->setStart(position);
            mobilityVisualization->velocityFigure->setEnd(position + velocity * velocityArrowScale);
            // hide when the projected arrow has no length (e.g. a (near-)vertical velocity on the map)
            mobilityVisualization->velocityFigure->setVisible(velocity.getLength() != 0);
        }
        if (displayMovementTrails)
            extendMovementTrail(mobility, mobilityVisualization, position);
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(mobilityVisualizations.empty() ? 0 : animationSpeed, this);
}

void MobilityCanvasVisualizer::addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization)
{
    MobilityVisualizerBase::addMobilityVisualization(mobility, mobilityVisualization);
    auto mobilityCanvasVisualization = static_cast<MobilityCanvasVisualization *>(mobilityVisualization);
    if (displayPositions)
        mobilityGroup->addFigure(mobilityCanvasVisualization->positionFigure);
    if (displayOrientations)
        mobilityGroup->addFigure(mobilityCanvasVisualization->orientationFigure);
    if (displayVelocities)
        mobilityGroup->addFigure(mobilityCanvasVisualization->velocityFigure);
    if (displayMovementTrails)
        mobilityGroup->addFigure(mobilityCanvasVisualization->trailFigure);
}

void MobilityCanvasVisualizer::removeMobilityVisualization(const MobilityVisualization *mobilityVisualization)
{
    auto mobilityCanvasVisualization = static_cast<const MobilityCanvasVisualization *>(mobilityVisualization);
    if (displayPositions)
        mobilityGroup->removeFigure(mobilityCanvasVisualization->positionFigure);
    if (displayOrientations)
        mobilityGroup->removeFigure(mobilityCanvasVisualization->orientationFigure);
    if (displayVelocities)
        mobilityGroup->removeFigure(mobilityCanvasVisualization->velocityFigure);
    if (displayMovementTrails)
        mobilityGroup->removeFigure(mobilityCanvasVisualization->trailFigure);
    MobilityVisualizerBase::removeMobilityVisualization(mobilityVisualization);
}

MobilityCanvasVisualizer::MobilityVisualization *MobilityCanvasVisualizer::createMobilityVisualization(IMobility *mobility)
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
    cOvalFigure *positionFigure = nullptr;
    if (displayPositions) {
        positionFigure = new cOvalFigure("position");
        positionFigure->setTags((std::string("position ") + tags).c_str());
        positionFigure->setTooltip("This circle represents the current position of the mobility model");
        positionFigure->setLineColor(positionCircleLineColorSet.getColor(module->getId()));
        positionFigure->setLineWidth(positionCircleLineWidth);
        positionFigure->setFilled(true);
        positionFigure->setFillColor(positionCircleFillColorSet.getColor(module->getId()));
    }
    cGroupFigure *orientationFigure = nullptr;
    cOvalFigure *orientationBaseFigure = nullptr;
    cLineFigure *orientationLineFigure[2] = { nullptr, nullptr };
    if (displayOrientations) {
        // a 3D orientation cone, drawn as its (rotated) base ellipse plus the two cone-side lines;
        // geometry is recomputed per frame in refreshDisplay()
        orientationFigure = new cGroupFigure("orientation");
        orientationBaseFigure = new cOvalFigure("orientationBase");
        orientationBaseFigure->setTags((std::string("orientation ") + tags).c_str());
        orientationBaseFigure->setTooltip("This ellipse is the base of the 3D orientation cone; it can be filled with a different color depending on whether the cone points up or down");
        orientationBaseFigure->setLineColor(orientationLineColor);
        orientationBaseFigure->setLineStyle(orientationLineStyle);
        orientationBaseFigure->setLineWidth(orientationLineWidth);
        orientationBaseFigure->setLineOpacity(orientationPieOpacity);
        orientationBaseFigure->setFillOpacity(orientationPieOpacity);
        // fill color and whether the disc is filled depend on the up/down direction, set per frame
        orientationFigure->addFigure(orientationBaseFigure);
        for (int i = 0; i < 2; i++) {
            auto line = new cLineFigure("orientationSide");
            line->setTags((std::string("orientation ") + tags).c_str());
            line->setLineColor(orientationLineColor);
            line->setLineStyle(orientationLineStyle);
            line->setLineWidth(orientationLineWidth);
            line->setLineOpacity(orientationPieOpacity);
            line->setVisible(false);
            orientationLineFigure[i] = line;
            orientationFigure->addFigure(line);
        }
    }
    cLineFigure *velocityFigure = nullptr;
    if (displayVelocities) {
        velocityFigure = new cLineFigure("velocity");
        velocityFigure->setTags((std::string("velocity ") + tags).c_str());
        velocityFigure->setTooltip("This arrow represents the current velocity of the mobility model");
        velocityFigure->setVisible(false);
        velocityFigure->setEndArrowhead(cFigure::ARROW_SIMPLE);
        velocityFigure->setLineColor(velocityLineColor);
        velocityFigure->setLineStyle(velocityLineStyle);
        velocityFigure->setLineWidth(velocityLineWidth);
    }
    TrailFigure *trailFigure = nullptr;
    if (displayMovementTrails) {
        trailFigure = new TrailFigure(trailLength, true, "movement trail");
        trailFigure->setTags((std::string("movement_trail recent_history ") + tags).c_str());
    }
    auto visualization = new MobilityCanvasVisualization(positionFigure, orientationFigure, velocityFigure, trailFigure, mobility);
    visualization->orientationBaseFigure = orientationBaseFigure;
    visualization->orientationLineFigure[0] = orientationLineFigure[0];
    visualization->orientationLineFigure[1] = orientationLineFigure[1];
    return visualization;
}

void MobilityCanvasVisualizer::extendMovementTrail(const IMobility *mobility, MobilityCanvasVisualization *visualization, cFigure::Point position) const
{
    auto trailFigure = visualization->trailFigure;
    // the chain is continued from the last trail position
    cFigure::Point startPosition = visualization->hasLastTrailPosition ? visualization->lastTrailPosition : position;
    cFigure::Point endPosition = position;
    double dx = startPosition.x - endPosition.x;
    double dy = startPosition.y - endPosition.y;
    // TODO 1?
    if (!visualization->hasLastTrailPosition || dx * dx + dy * dy > 1) {
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
        // hide trail discontinuities: an over-long segment (e.g. a node wrapping across the map
        // on an equirectangular projection) is treated as a jump and hidden, so the visible trail breaks
        // into disjoint pieces. The segment is still added so the trail (and its length window) continues
        // past the jump.
        if (dx * dx + dy * dy > movementTrailSegmentHideThreshold * movementTrailSegmentHideThreshold)
            movementLine->setVisible(false);
        trailFigure->addFigure(movementLine);
        visualization->lastTrailPosition = position;
        visualization->hasLastTrailPosition = true;
    }
}

} // namespace visualizer

} // namespace inet

