//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/environment/common/PhysicalEnvironment.h"
#include "inet/common/geometry/object/Box.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/shape/Sphere.h"
#include "inet/common/geometry/shape/Prism.h"
#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/common/geometry/common/Rotation.h"
#include <algorithm>

namespace inet {

namespace physicalenvironment {

Define_Module(PhysicalEnvironment);

PhysicalEnvironment::PhysicalEnvironment() :
    temperature(NaN),
    spaceMin(Coord(NaN, NaN, NaN)),
    spaceMax(Coord(NaN, NaN, NaN)),
    axisLength(NaN),
    objectCache(nullptr),
    objectsLayer(nullptr)
{
}

PhysicalEnvironment::~PhysicalEnvironment()
{
    for (auto & elem : shapes)
        delete elem;
    for (auto & elem : materials)
        delete elem;
    for (auto & elem : objects)
        delete elem;
}

void PhysicalEnvironment::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        objectCache = dynamic_cast<IObjectCache *>(getSubmodule("objectCache"));
        temperature = K(par("temperature"));
        spaceMin.x = par("spaceMinX");
        spaceMin.y = par("spaceMinY");
        spaceMin.z = par("spaceMinZ");
        spaceMax.x = par("spaceMaxX");
        spaceMax.y = par("spaceMaxY");
        spaceMax.z = par("spaceMaxZ");
        axisLength = par("axisLength");
        viewAngle = computeViewAngle(par("viewAngle"));
        viewRotation = Rotation(viewAngle);
        viewTranslation = computeViewTranslation(par("viewTranslation"));
        objectsLayer = new cGroupFigure();
        cCanvas *canvas = getParentModule()->getCanvas();
        canvas->addFigureBelow(objectsLayer, canvas->getSubmodulesLayer());
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
    {
        cXMLElement *environment = par("config");
        parseShapes(environment);
        parseMaterials(environment);
        parseObjects(environment);
    }
    else if (stage == INITSTAGE_LAST)
    {
        updateCanvas();
    }
}

void PhysicalEnvironment::parseShapes(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("shape");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        cXMLElement *element = *it;
        ShapeBase *shape = nullptr;
        const char *tok;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // type
        const char *typeAttribute = element->getAttribute("type");
        if (!typeAttribute)
            throw cRuntimeError("Missing type attribute of shape");
        else if (!strcmp(typeAttribute, "cuboid"))
        {
            Coord size;
            const char *sizeAttribute = element->getAttribute("size");
            if (!sizeAttribute)
                throw cRuntimeError("Missing size attribute of cuboid");
            cStringTokenizer tokenizer(sizeAttribute);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid size x at %s", element->getSourceLocation());
            size.x = atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid size y at %s", element->getSourceLocation());
            size.y = atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid size z at %s", element->getSourceLocation());
            size.z = atof(tok);
            shape = new Cuboid(size);
        }
        else if (!strcmp(typeAttribute, "sphere"))
        {
            double radius;
            const char *radiusAttribute = element->getAttribute("radius");
            if (!radiusAttribute)
                throw cRuntimeError("Missing radius attribute of sphere");
            radius = atof(radiusAttribute);
            shape = new Sphere(radius);
        }
        else if (!strcmp(typeAttribute, "prism"))
        {
            double height;
            const char *heightAttribute = element->getAttribute("height");
            if (!heightAttribute)
                throw cRuntimeError("Missing height attribute of prism");
            height = atof(heightAttribute);
            std::vector<Coord> points;
            const char *pointsAttribute = element->getAttribute("points");
            if (!pointsAttribute)
                throw cRuntimeError("Missing points attribute of prism");

            cStringTokenizer tokenizer(pointsAttribute);
            while (tokenizer.hasMoreTokens())
            {
                Coord point;
                point.x = atof(tokenizer.nextToken());
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing prism y at %s", element->getSourceLocation());
                point.y = atof(tok);
                points.push_back(point);
            }
            if (points.size() < 3)
                throw cRuntimeError("prism needs at least three points at %s", element->getSourceLocation());
            shape = new Prism(height, Polygon(points));
        }
        else if (!strcmp(typeAttribute, "polyhedron"))
        {
            std::vector<Coord> points;
            const char *pointsAttribute = element->getAttribute("points");
            if (!pointsAttribute)
                throw cRuntimeError("Missing points attribute of polyhedron");

            cStringTokenizer tokenizer(pointsAttribute);
            while (tokenizer.hasMoreTokens())
            {
                Coord point;
                point.x = atof(tokenizer.nextToken());
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing polyhedron y at %s", element->getSourceLocation());
                point.y = atof(tok);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing polyhedron z at %s", element->getSourceLocation());
                point.z = atof(tok);
                points.push_back(point);
            }
            if (points.size() < 4)
                throw cRuntimeError("polyhedron needs at least four points at %s", element->getSourceLocation());
            shape = new Polyhedron(points);
        }
        else
            throw cRuntimeError("Unknown shape type '%s'", typeAttribute);
        // insert
        if (idToShapeMap.find(id) != idToShapeMap.end())
            throw cRuntimeError("Shape already exists with the same id: '%d'", id);
        idToShapeMap.insert(std::pair<int, const ShapeBase *>(id, shape));
    }
}

void PhysicalEnvironment::parseMaterials(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("material");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        cXMLElement *element = *it;
        // id
        const char *idAttribute = element->getAttribute("id");
        if (!idAttribute)
            throw cRuntimeError("Missing mandatory id attribute of material");
        int id = atoi(idAttribute);
        // name
        const char *name = element->getAttribute("name");
        // resistivity
        const char *resistivityAttribute = element->getAttribute("resistivity");
        if (!resistivityAttribute)
            throw cRuntimeError("Missing mandatory resistivity attribute of material");
        Ohmm resistivity = Ohmm(atof(resistivityAttribute));
        // relativePermittivity
        const char *relativePermittivityAttribute = element->getAttribute("relativePermittivity");
        if (!relativePermittivityAttribute)
            throw cRuntimeError("Missing mandatory relativePermittivity attribute of material");
        double relativePermittivity = atof(relativePermittivityAttribute);
        // relativePermeability
        const char *relativePermeabilityAttribute = element->getAttribute("relativePermeability");
        if (!relativePermeabilityAttribute)
            throw cRuntimeError("Missing mandatory relativePermeability attribute of material");
        double relativePermeability = atof(relativePermeabilityAttribute);
        // insert
        if (idToMaterialMap.find(id) != idToMaterialMap.end())
            throw cRuntimeError("Material already exists with the same id: '%d'", id);
        Material *material = new Material(name, resistivity, relativePermittivity, relativePermeability);
        materials.push_back(material);
        idToMaterialMap.insert(std::pair<int, const Material *>(id, material));
        nameToMaterialMap.insert(std::pair<const std::string, const Material *>(material->getName(), material));
    }
}

void PhysicalEnvironment::parseObjects(cXMLElement *xml)
{
    Coord computedSpaceMin = Coord::NIL;
    Coord computedSpaceMax = Coord::NIL;;
    cXMLElementList children = xml->getChildren();
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        const char *tok;
        cXMLElement *element = *it;
        const char *tag = element->getTagName();
        if (strcmp(tag, "object"))
            continue;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // name
        const char *name = element->getAttribute("name");
        // orientation
        EulerAngles orientation;
        const char *orientationAttribute = element->getAttribute("orientation");
        if (orientationAttribute)
        {
            cStringTokenizer tokenizer(orientationAttribute);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation alpha at %s", element->getSourceLocation());
            orientation.alpha = math::deg2rad(atof(tok));
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation beta at %s", element->getSourceLocation());
            orientation.beta = math::deg2rad(atof(tok));
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation gamma at %s", element->getSourceLocation());
            orientation.gamma = math::deg2rad(atof(tok));
        }
        // shape
        const ShapeBase *shape = nullptr;
        const char *shapeAttribute = element->getAttribute("shape");
        if (!shapeAttribute)
            throw cRuntimeError("Missing shape attribute of object");
        cStringTokenizer shapeTokenizer(shapeAttribute);
        const char *shapeType = shapeTokenizer.nextToken();
        if (!strcmp(shapeType, "cuboid"))
        {
            Coord size;
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid x at %s", element->getSourceLocation());
            size.x = atof(tok);
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid y at %s", element->getSourceLocation());
            size.y = atof(tok);
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing cuboid z at %s", element->getSourceLocation());
            size.z = atof(tok);
            shape = new Cuboid(size);
            shapes.push_back(shape);
        }
        else if (!strcmp(shapeType, "sphere"))
        {
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing sphere radius at %s", element->getSourceLocation());
            double radius = atof(tok);
            shape = new Sphere(radius);
            shapes.push_back(shape);
        }
        else if (!strcmp(shapeType, "prism"))
        {
            double height;
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing prism height at %s", element->getSourceLocation());
            height = atof(tok);
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens())
            {
                Coord point;
                point.x = atof(shapeTokenizer.nextToken());
                if ((tok = shapeTokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing prism y at %s", element->getSourceLocation());
                point.y = atof(tok);
                points.push_back(point);
            }
            if (points.size() < 3)
                throw cRuntimeError("prism needs at least three points at %s", element->getSourceLocation());
            Box boundingBox = Box::computeBoundingBox(points);
            Coord center = (boundingBox.getMax() - boundingBox.getMin()) / 2 + boundingBox.getMin();
            center.z = height / 2;
            std::vector<Coord> prismPoints;
            for (auto & point : points)
                prismPoints.push_back(point - center);
            shape = new Prism(height, Polygon(prismPoints));
            shapes.push_back(shape);
        }
        else if (!strcmp(shapeType, "polyhedron"))
        {
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens())
            {
                Coord point;
                point.x = atof(shapeTokenizer.nextToken());
                if ((tok = shapeTokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing polyhedron y at %s", element->getSourceLocation());
                point.y = atof(tok);
                if ((tok = shapeTokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing polyhedron z at %s", element->getSourceLocation());
                point.z = atof(tok);
                points.push_back(point);
            }
            if (points.size() < 4)
                throw cRuntimeError("polyhedron needs at least four points at %s", element->getSourceLocation());
            Box boundingBox = Box::computeBoundingBox(points);
            Coord center = (boundingBox.getMax() - boundingBox.getMin()) / 2 + boundingBox.getMin();
            std::vector<Coord> PolyhedronPoints;
            for (auto & point : points)
                PolyhedronPoints.push_back(point - center);
            shape = new Polyhedron(PolyhedronPoints);
            shapes.push_back(shape);
        }
        else {
            int id = atoi(shapeAttribute);
            shape = idToShapeMap[id];
        }
        if (!shape)
            throw cRuntimeError("Unknown shape '%s'", shapeAttribute);
        const Coord size = shape->computeBoundingBoxSize();
        // position
        Coord position = Coord::NIL;
        const char *positionAttribute = element->getAttribute("position");
        if (positionAttribute)
        {
            cStringTokenizer tokenizer(positionAttribute);
            const char *kind = tokenizer.nextToken();
            if (!kind)
                throw cRuntimeError("Missing position kind");
            else if (!strcmp(kind, "min"))
                position = size / 2;
            else if (!strcmp(kind, "max"))
                position = size / -2;
            else if (!strcmp(kind, "center"))
                position = Coord::ZERO;
            else
                throw cRuntimeError("Unknown position kind");
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position x at %s", element->getSourceLocation());
            position.x += atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position y at %s", element->getSourceLocation());
            position.y += atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position z at %s", element->getSourceLocation());
            position.z += atof(tok);
        }
        // material
        const Material *material;
        const char *materialAttribute = element->getAttribute("material");
        if (!materialAttribute)
            throw cRuntimeError("Missing material attribute of object");
        else if (nameToMaterialMap.find(materialAttribute) != nameToMaterialMap.end())
            material = nameToMaterialMap[materialAttribute];
        else if (MaterialRegistry::singleton.getMaterial(materialAttribute))
            material = MaterialRegistry::singleton.getMaterial(materialAttribute);
        else
            material = idToMaterialMap[atoi(materialAttribute)];
        if (!material)
            throw cRuntimeError("Unknown material '%s'", materialAttribute);
        // line width
        double lineWidth = 1;
        const char *lineWidthAttribute = element->getAttribute("line-width");
        if (lineWidthAttribute)
            lineWidth = atof(lineWidthAttribute);
        // line color
        cFigure::Color lineColor = cFigure::BLACK;
        const char *lineColorAttribute = element->getAttribute("line-color");
        if (lineColorAttribute)
        {
            if (strchr(lineColorAttribute, ' '))
            {
                cStringTokenizer tokenizer(lineColorAttribute);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing line-color red at %s", element->getSourceLocation());
                lineColor.red = atoi(tok);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing line-color green at %s", element->getSourceLocation());
                lineColor.green = atoi(tok);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing line-color blue at %s", element->getSourceLocation());
                lineColor.blue = atoi(tok);
            }
            else
#if OMNETPP_VERSION >= 0x500
                lineColor = cFigure::Color(lineColorAttribute);
#else
                lineColor = cFigure::Color::byName(lineColorAttribute);
#endif
        }
        // fill color
        cFigure::Color fillColor = cFigure::WHITE;
        const char *fillColorAttribute = element->getAttribute("fill-color");
        if (fillColorAttribute)
        {
            if (strchr(fillColorAttribute, ' '))
            {
                cStringTokenizer tokenizer(fillColorAttribute);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing fill-color red at %s", element->getSourceLocation());
                fillColor.red = atoi(tok);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing fill-color green at %s", element->getSourceLocation());
                fillColor.green = atoi(tok);
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing fill-color blue at %s", element->getSourceLocation());
                fillColor.blue = atoi(tok);
            }
            else
#if OMNETPP_VERSION >= 0x500
                fillColor = cFigure::Color(fillColorAttribute);
#else
                fillColor = cFigure::Color::byName(fillColorAttribute);
#endif
        }
        // opacity
        double opacity = 1;
        const char *opacityAttribute = element->getAttribute("opacity");
        if (opacityAttribute)
            opacity = atof(opacityAttribute);
        // tags
        const char *tags = element->getAttribute("tags");
        // insert object
        PhysicalObject *object = new PhysicalObject(name, id, position, orientation, shape, material, lineWidth, lineColor, fillColor, opacity, tags);
        objects.push_back(object);
        if (id != -1)
            idToObjectMap.insert(std::pair<int, const PhysicalObject *>(id, object));
        if (objectCache)
            objectCache->insertObject(object);
        const Coord min = position - size / 2;
        const Coord max = position + size / 2;
        if ((!std::isnan(spaceMin.x) && min.x < spaceMin.x) || (!std::isnan(spaceMax.x) && max.x > spaceMax.x) ||
            (!std::isnan(spaceMin.y) && min.y < spaceMin.y) || (!std::isnan(spaceMax.y) && max.y > spaceMax.y) ||
            (!std::isnan(spaceMin.z) && min.z < spaceMin.z) || (!std::isnan(spaceMax.z) && max.z > spaceMax.z))
            throw cRuntimeError("Object is outside of space limits");
        if (std::isnan(computedSpaceMin.x) || min.x < computedSpaceMin.x) computedSpaceMin.x = min.x;
        if (std::isnan(computedSpaceMin.y) || min.y < computedSpaceMin.y) computedSpaceMin.y = min.y;
        if (std::isnan(computedSpaceMin.z) || min.z < computedSpaceMin.z) computedSpaceMin.z = min.z;
        if (std::isnan(computedSpaceMax.x) || max.x > computedSpaceMax.x) computedSpaceMax.x = max.x;
        if (std::isnan(computedSpaceMax.y) || max.y > computedSpaceMax.y) computedSpaceMax.y = max.y;
        if (std::isnan(computedSpaceMax.z) || max.z > computedSpaceMax.z) computedSpaceMax.z = max.z;
    }
    if (std::isnan(spaceMin.x)) spaceMin.x = computedSpaceMin.x;
    if (std::isnan(spaceMin.y)) spaceMin.y = computedSpaceMin.y;
    if (std::isnan(spaceMin.z)) spaceMin.z = computedSpaceMin.z;
    if (std::isnan(spaceMax.x)) spaceMax.x = computedSpaceMax.x;
    if (std::isnan(spaceMax.y)) spaceMax.y = computedSpaceMax.y;
    if (std::isnan(spaceMax.z)) spaceMax.z = computedSpaceMax.z;
}

void PhysicalEnvironment::updateCanvas()
{
    while (objectsLayer->getNumFigures())
        delete objectsLayer->removeFigure(0);
    // KLUDGE: TODO: sorting objects with their rotated position's z coordinate to draw them in a "better" order
    std::vector<const PhysicalObject *> objectsCopy = objects;
    std::stable_sort(objectsCopy.begin(), objectsCopy.end(), ObjectPositionComparator(viewRotation));
    for (auto object : objectsCopy)
    {
        const ShapeBase *shape = object->getShape();
        const Coord& position = object->getPosition();
        const EulerAngles& orientation = object->getOrientation();
        const Rotation rotation(orientation);
        // cuboid
        const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
        if (cuboid)
        {
            std::vector<std::vector<Coord> > faces;
            cuboid->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // sphere
        const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
        if (sphere)
        {
            double radius = sphere->getRadius();
            cOvalFigure *figure = new cOvalFigure();
            figure->setFilled(true);
            cFigure::Point center = computeCanvasPoint(position, viewRotation, viewTranslation);
            figure->setBounds(cFigure::Rectangle(center.x - radius, center.y - radius, radius * 2, radius * 2));
            figure->setLineWidth(object->getLineWidth());
            figure->setLineColor(object->getLineColor());
            figure->setFillColor(object->getFillColor());
#if OMNETPP_VERSION >= 0x500
            figure->setLineOpacity(object->getOpacity());
            figure->setFillOpacity(object->getOpacity());
            figure->setZoomLineWidth(false);
#endif
            std::string tags("physical_object ");
            if (object->getTags())
                tags += object->getTags();
            figure->setTags(tags.c_str());
            objectsLayer->addFigure(figure);
        }
        // prism
        const Prism *prism = dynamic_cast<const Prism *>(shape);
        if (prism)
        {
            std::vector<std::vector<Coord> > faces;
            prism->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // polyhedron
        const Polyhedron *polyhedron = dynamic_cast<const Polyhedron *>(shape);
        if (polyhedron)
        {
            std::vector<std::vector<Coord> > faces;
            polyhedron->computeVisibleFaces(faces, rotation, viewRotation);
            computeFacePoints(object, faces, rotation);
        }
        // add name to the end
        const char *name = object->getName();
        if (name)
        {
#if OMNETPP_VERSION >= 0x500
            cLabelFigure *nameFigure = new cLabelFigure();
            nameFigure->setPosition(computeCanvasPoint(position, viewRotation, viewTranslation));
#else
            cTextFigure *nameFigure = new cTextFigure();
            nameFigure->setLocation(computeCanvasPoint(position, viewRotation, viewTranslation));
#endif
            nameFigure->setTags("physical_object object_name label");
            nameFigure->setText(name);
            objectsLayer->addFigure(nameFigure);
        }
    }
    if (!std::isnan(axisLength)) {
        cLineFigure *xAxis = new cLineFigure();
        cLineFigure *yAxis = new cLineFigure();
        cLineFigure *zAxis = new cLineFigure();
        xAxis->setTags("axis");
        yAxis->setTags("axis");
        zAxis->setTags("axis");
        xAxis->setLineWidth(1);
        yAxis->setLineWidth(1);
        zAxis->setLineWidth(1);
#if OMNETPP_VERSION >= 0x500 && OMNETPP_BUILDNUM >= 1006
        xAxis->setEndArrowhead(cFigure::ARROW_BARBED);
        yAxis->setEndArrowhead(cFigure::ARROW_BARBED);
        zAxis->setEndArrowhead(cFigure::ARROW_BARBED);
#else
        xAxis->setEndArrowHead(cFigure::ARROW_BARBED);
        yAxis->setEndArrowHead(cFigure::ARROW_BARBED);
        zAxis->setEndArrowHead(cFigure::ARROW_BARBED);
#endif
#if OMNETPP_VERSION >= 0x500
        xAxis->setZoomLineWidth(false);
        yAxis->setZoomLineWidth(false);
        zAxis->setZoomLineWidth(false);
#endif
        xAxis->setStart(computeCanvasPoint(Coord::ZERO));
        yAxis->setStart(computeCanvasPoint(Coord::ZERO));
        zAxis->setStart(computeCanvasPoint(Coord::ZERO));
        xAxis->setEnd(computeCanvasPoint(Coord(axisLength, 0, 0)));
        yAxis->setEnd(computeCanvasPoint(Coord(0, axisLength, 0)));
        zAxis->setEnd(computeCanvasPoint(Coord(0, 0, axisLength)));
        objectsLayer->addFigure(xAxis);
        objectsLayer->addFigure(yAxis);
        objectsLayer->addFigure(zAxis);
#if OMNETPP_VERSION >= 0x500
        cLabelFigure *xLabel = new cLabelFigure();
        cLabelFigure *yLabel = new cLabelFigure();
        cLabelFigure *zLabel = new cLabelFigure();
#else
        cTextFigure *xLabel = new cTextFigure();
        cTextFigure *yLabel = new cTextFigure();
        cTextFigure *zLabel = new cTextFigure();
#endif
        xLabel->setTags("axis label");
        yLabel->setTags("axis label");
        zLabel->setTags("axis label");
        xLabel->setText("X");
        yLabel->setText("Y");
        zLabel->setText("Z");
#if OMNETPP_VERSION >= 0x500
        xLabel->setPosition(computeCanvasPoint(Coord(axisLength, 0, 0)));
        yLabel->setPosition(computeCanvasPoint(Coord(0, axisLength, 0)));
        zLabel->setPosition(computeCanvasPoint(Coord(0, 0, axisLength)));
#else
        xLabel->setLocation(computeCanvasPoint(Coord(axisLength, 0, 0)));
        yLabel->setLocation(computeCanvasPoint(Coord(0, axisLength, 0)));
        zLabel->setLocation(computeCanvasPoint(Coord(0, 0, axisLength)));
#endif
        objectsLayer->addFigure(xLabel);
        objectsLayer->addFigure(yLabel);
        objectsLayer->addFigure(zLabel);
    }
}

void PhysicalEnvironment::computeFacePoints(const PhysicalObject *object, std::vector<std::vector<Coord> >& faces, const Rotation& rotation)
{
    const Coord& position = object->getPosition();
    for (std::vector<std::vector<Coord> >::const_iterator it = faces.begin(); it != faces.end(); it++)
    {
        std::vector<cFigure::Point> canvasPoints;
        const std::vector<Coord>& facePoints = *it;
        for (const auto & facePoint : facePoints)
        {
            cFigure::Point canvPoint = computeCanvasPoint(rotation.rotateVectorClockwise(facePoint) + position, viewRotation, viewTranslation);
            canvasPoints.push_back(canvPoint);
        }
        cPolygonFigure *figure = new cPolygonFigure();
        figure->setFilled(true);
        figure->setPoints(canvasPoints);
        figure->setLineWidth(object->getLineWidth());
        figure->setLineColor(object->getLineColor());
        figure->setFillColor(object->getFillColor());
#if OMNETPP_VERSION >= 0x500
        figure->setLineOpacity(object->getOpacity());
        figure->setFillOpacity(object->getOpacity());
        figure->setZoomLineWidth(false);
#endif
        std::string tags("physical_object ");
        if (object->getTags())
            tags += object->getTags();
        figure->setTags(tags.c_str());
        objectsLayer->addFigure(figure);
    }
}

const PhysicalObject *PhysicalEnvironment::getObjectById(int id) const
{
    std::map<int, const PhysicalObject *>::const_iterator it = idToObjectMap.find(id);
    if (it == idToObjectMap.end())
        return nullptr;
    else
        return it->second;
}

void PhysicalEnvironment::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    if (objectCache)
        objectCache->visitObjects(visitor, lineSegment);
    else
        for (const auto & elem : objects)
            visitor->visit(elem);
}

void PhysicalEnvironment::handleParameterChange(const char* name)
{
    if (name && !strcmp(name, "viewAngle")) {
        viewAngle = computeViewAngle(par("viewAngle"));
        viewRotation = Rotation(viewAngle);
        updateCanvas();
    }
    else if (name && !strcmp(name, "viewTranslation")) {
        viewTranslation = computeViewTranslation(par("viewTranslation"));
        updateCanvas();
    }
}

EulerAngles PhysicalEnvironment::computeViewAngle(const char* viewAngle)
{
    double x, y, z;
    if (!strcmp(viewAngle, "x"))
    {
        x = 0;
        y = M_PI / 2;
        z = M_PI / -2;
    }
    else if (!strcmp(viewAngle, "y"))
    {
        x = M_PI / 2;
        y = 0;
        z = 0;
    }
    else if (!strcmp(viewAngle, "z"))
    {
        x = y = z = 0;
    }
    else if (!strncmp(viewAngle, "isometric", 9))
    {
        int v;
        int l = strlen(viewAngle);
        switch (l) {
            case 9: v = 0; break;
            case 10: v = viewAngle[9] - '0'; break;
            case 11: v = (viewAngle[9] - '0') * 10 + viewAngle[10] - '0'; break;
            default: throw cRuntimeError("Invalid isometric viewAngle parameter");
        }
        // 1st axis can point on the 2d plane in 6 directions
        // 2nd axis can point on the 2d plane in 4 directions (the opposite direction is forbidden)
        // 3rd axis can point on the 2d plane in 2 directions
        // this results in 6 * 4 * 2 = 48 different configurations
        x = math::deg2rad(45 + v % 4 * 90);
        y = math::deg2rad(v / 24 % 2 ? 35.27 : -35.27);
        z = math::deg2rad(30 + v / 4 % 6 * 60);
    }
    else if (sscanf(viewAngle, "%lf %lf %lf", &x, &y, &z) == 3)
    {
        x = math::deg2rad(x);
        y = math::deg2rad(y);
        z = math::deg2rad(z);
    }
    else
        throw cRuntimeError("The viewAngle parameter must be a predefined string or a triplet representing three degrees");
    return EulerAngles(x, y, z);
}

cFigure::Point PhysicalEnvironment::computeViewTranslation(const char* viewTranslation)
{
    double x, y;
    if (sscanf(viewTranslation, "%lf %lf", &x, &y) == 2)
    {
        x = math::deg2rad(x);
        y = math::deg2rad(y);
    }
    else
        throw cRuntimeError("The viewTranslation parameter must be a pair of doubles");
    return cFigure::Point(x, y);
}

} // namespace physicalenvironment

} // namespace inet

