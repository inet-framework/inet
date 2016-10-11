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
#include "inet/visualizer/physicallayer/obstacleloss/TracingObstacleLossCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(TracingObstacleLossCanvasVisualizer);

void TracingObstacleLossCanvasVisualizer::initialize(int stage)
{
    TracingObstacleLossVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        cCanvas *canvas = visualizerTargetModule->getCanvas();
        intersectionTrail = new TrailFigure(100, true, "obstacle intersection trail");
        intersectionTrail->setZIndex(zIndex);
        canvas->addFigureBelow(intersectionTrail, canvas->getSubmodulesLayer());
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
        canvasProjection = CanvasProjection::getCanvasProjection(visualizerTargetModule->getCanvas());
}

void TracingObstacleLossCanvasVisualizer::obstaclePenetrated(const IPhysicalObject *object, const Coord& intersection1, const Coord& intersection2, const Coord& normal1, const Coord& normal2)
{
    if (displayIntersectionTrail || displayFaceNormalVectorTrail) {
        const Rotation rotation(object->getOrientation());
        const Coord& position = object->getPosition();
        const Coord rotatedIntersection1 = rotation.rotateVectorClockwise(intersection1);
        const Coord rotatedIntersection2 = rotation.rotateVectorClockwise(intersection2);
        double intersectionDistance = intersection2.distance(intersection1);
        if (displayIntersectionTrail) {
            cLineFigure *intersectionLine = new cLineFigure("intersection");
            intersectionLine->setTags("obstacle_intersection recent_history");
            intersectionLine->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position));
            intersectionLine->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position));
            intersectionLine->setLineColor(cFigure::RED);
            intersectionLine->setLineWidth(1);
            intersectionTrail->addFigure(intersectionLine);
            intersectionLine->setZoomLineWidth(false);
        }
        if (displayFaceNormalVectorTrail) {
            Coord normalVisualization1 = normal1 / normal1.length() * intersectionDistance / 10;
            Coord normalVisualization2 = normal2 / normal2.length() * intersectionDistance / 10;
            cLineFigure *normal1Line = new cLineFigure("normal1");
            normal1Line->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position));
            normal1Line->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection1 + position + rotation.rotateVectorClockwise(normalVisualization1)));
            normal1Line->setLineColor(cFigure::GREY);
            normal1Line->setTags("obstacle_intersection face_normal_vector recent_history");
            normal1Line->setLineWidth(1);
            intersectionTrail->addFigure(normal1Line);
            cLineFigure *normal2Line = new cLineFigure("normal2");
            normal2Line->setStart(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position));
            normal2Line->setEnd(canvasProjection->computeCanvasPoint(rotatedIntersection2 + position + rotation.rotateVectorClockwise(normalVisualization2)));
            normal2Line->setLineColor(cFigure::GREY);
            normal2Line->setTags("obstacle_intersection face_normal_vector recent_history");
            normal2Line->setLineWidth(1);
            intersectionTrail->addFigure(normal2Line);
            normal1Line->setZoomLineWidth(false);
            normal2Line->setZoomLineWidth(false);
        }
    }
}

} // namespace visualizer

} // namespace inet

