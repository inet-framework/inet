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

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/shape/Prism.h"
#include "inet/common/geometry/shape/Sphere.h"
#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/visualizer/environment/PhysicalEnvironmentCanvasVisualizer.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

Define_Module(PhysicalEnvironmentCanvasVisualizer);

void PhysicalEnvironmentCanvasVisualizer::initialize(int stage)
{
    PhysicalEnvironmentVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        cCanvas *canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
        objectsLayer = new cGroupFigure("objectsLayer");
        objectsLayer->setZIndex(zIndex);
        objectsLayer->insertBelow(canvas->getSubmodulesLayer());
    }
}

void PhysicalEnvironmentCanvasVisualizer::refreshDisplay() const
{
    // only update after initialize
    if (physicalEnvironment != nullptr && getSimulation()->getEventNumber() == 0 && displayObjects) {
        while (objectsLayer->getNumFigures())
            delete objectsLayer->removeFigure(0);
        // KLUDGE: sorting objects with their rotated position's z coordinate to draw them in a "better" order
        std::vector<const IPhysicalObject *> objectsCopy;
        for (int i = 0; i < physicalEnvironment->getNumObjects(); i++)
            objectsCopy.push_back(physicalEnvironment->getObject(i));
        std::stable_sort(objectsCopy.begin(), objectsCopy.end(), ObjectPositionComparator(canvasProjection->getRotation()));
        for (auto object : objectsCopy) {
            const ShapeBase *shape = object->getShape();
            const Coord& position = object->getPosition();
            const Quaternion& orientation = object->getOrientation();
            const RotationMatrix rotation(orientation.toEulerAngles());
            // cuboid
            const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
            if (cuboid) {
                std::vector<std::vector<Coord> > faces;
                cuboid->computeVisibleFaces(faces, rotation, canvasProjection->getRotation());
                computeFacePoints(object, faces, rotation);
            }
            // sphere
            const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
            if (sphere) {
                double radius = sphere->getRadius();
                cOvalFigure *figure = new cOvalFigure("sphere");
                figure->setTooltip("This oval represents a physical object");
                figure->setAssociatedObject(const_cast<cObject *>(check_and_cast<const cObject *>(object)));
                figure->setFilled(true);
                cFigure::Point center = canvasProjection->computeCanvasPoint(position);
                figure->setBounds(cFigure::Rectangle(center.x - radius, center.y - radius, radius * 2, radius * 2));
                figure->setLineWidth(object->getLineWidth());
                figure->setLineColor(object->getLineColor());
                figure->setFillColor(object->getFillColor());
                figure->setLineOpacity(object->getOpacity());
                figure->setFillOpacity(object->getOpacity());
                figure->setZoomLineWidth(false);
                std::string objectTags("physical_object ");
                if (object->getTags())
                    objectTags += object->getTags();
                figure->setTags((objectTags + " " + tags).c_str());
                objectsLayer->addFigure(figure);
            }
            // prism
            const Prism *prism = dynamic_cast<const Prism *>(shape);
            if (prism) {
                std::vector<std::vector<Coord> > faces;
                prism->computeVisibleFaces(faces, rotation, canvasProjection->getRotation());
                computeFacePoints(object, faces, rotation);
            }
            // polyhedron
            const Polyhedron *polyhedron = dynamic_cast<const Polyhedron *>(shape);
            if (polyhedron) {
                std::vector<std::vector<Coord> > faces;
                polyhedron->computeVisibleFaces(faces, rotation, canvasProjection->getRotation());
                computeFacePoints(object, faces, rotation);
            }
            // add name to the end
            const char *name = check_and_cast<const cObject *>(object)->getName();
            if (name) {
                cLabelFigure *nameFigure = new cLabelFigure("objectName");
                nameFigure->setPosition(canvasProjection->computeCanvasPoint(position));
                nameFigure->setTags((std::string("physical_object object_name label ") + tags).c_str());
                nameFigure->setText(name);
                objectsLayer->addFigure(nameFigure);
            }
        }
    }
}

void PhysicalEnvironmentCanvasVisualizer::computeFacePoints(const IPhysicalObject *object, std::vector<std::vector<Coord> >& faces, const RotationMatrix& rotation) const
{
    const Coord& position = object->getPosition();
    for (std::vector<std::vector<Coord> >::const_iterator it = faces.begin(); it != faces.end(); it++)
    {
        std::vector<cFigure::Point> canvasPoints;
        const std::vector<Coord>& facePoints = *it;
        for (const auto & facePoint : facePoints)
        {
            cFigure::Point canvPoint = canvasProjection->computeCanvasPoint(rotation.rotateVector(facePoint) + position);
            canvasPoints.push_back(canvPoint);
        }
        cPolygonFigure *figure = new cPolygonFigure("objectFace");
        figure->setTooltip("This polygon represents a physical object");
        figure->setAssociatedObject(const_cast<cObject *>(check_and_cast<const cObject *>(object)));
        figure->setFilled(true);
        figure->setPoints(canvasPoints);
        figure->setLineWidth(object->getLineWidth());
        figure->setLineColor(object->getLineColor());
        figure->setFillColor(object->getFillColor());
        figure->setLineOpacity(object->getOpacity());
        figure->setFillOpacity(object->getOpacity());
        figure->setZoomLineWidth(false);
        std::string objectTags("physical_object ");
        if (object->getTags())
            objectTags += object->getTags();
        figure->setTags((objectTags + " " + tags).c_str());
        objectsLayer->addFigure(figure);
    }
}

} // namespace visualizer

} // namespace inet

