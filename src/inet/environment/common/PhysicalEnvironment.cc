//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/common/PhysicalEnvironment.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/object/Box.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/common/geometry/shape/Prism.h"
#include "inet/common/geometry/shape/Sphere.h"
#include "inet/common/geometry/shape/polyhedron/Polyhedron.h"
#include "inet/common/stlutils.h"

namespace inet {

namespace physicalenvironment {

Define_Module(PhysicalEnvironment);

PhysicalEnvironment::PhysicalEnvironment() :
    temperature(NaN),
    spaceMin(Coord(NaN, NaN, NaN)),
    spaceMax(Coord(NaN, NaN, NaN)),
    objectCache(nullptr)
{
}

PhysicalEnvironment::~PhysicalEnvironment()
{
    for (auto& elem : shapes)
        delete elem;
    for (auto& elem : materials)
        delete elem;
    for (auto& elem : objects)
        delete elem;
}

void PhysicalEnvironment::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        objectCache = dynamic_cast<IObjectCache *>(getSubmodule("objectCache"));
        ground = dynamic_cast<IGround *>(getSubmodule("ground"));
        temperature = K(par("temperature"));
        spaceMin.x = par("spaceMinX");
        spaceMin.y = par("spaceMinY");
        spaceMin.z = par("spaceMinZ");
        spaceMax.x = par("spaceMaxX");
        spaceMax.y = par("spaceMaxY");
        spaceMax.z = par("spaceMaxZ");
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        cXMLElement *environment = par("config");
        parseShapes(environment);
        parseMaterials(environment);
        parseObjects(environment);
    }
}

void PhysicalEnvironment::convertPoints(std::vector<Coord>& points)
{
    auto originPosition = coordinateSystem == nullptr ? GeoCoord(deg(0), deg(0), m(0)) : coordinateSystem->computeGeographicCoordinate(Coord::ZERO);
    Box boundingBox = Box::computeBoundingBox(points);
    Coord center = boundingBox.getCenter();
    for (auto& point : points) {
        point -= center;
        if (coordinateSystem != nullptr)
            point = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(point.x) + originPosition.latitude, deg(point.y) + originPosition.longitude, m(0)));
    }
}

void PhysicalEnvironment::parseShapes(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("shape");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
        cXMLElement *element = *it;
        ShapeBase *shape = nullptr;
        const char *tok;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        if (containsKey(idToShapeMap, id))
            throw cRuntimeError("Shape already exists with the same id: '%d'", id);
        // type
        const char *typeAttribute = element->getAttribute("type");
        if (!typeAttribute)
            throw cRuntimeError("Missing type attribute of shape");
        else if (!strcmp(typeAttribute, "cuboid")) {
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
        else if (!strcmp(typeAttribute, "sphere")) {
            double radius;
            const char *radiusAttribute = element->getAttribute("radius");
            if (!radiusAttribute)
                throw cRuntimeError("Missing radius attribute of sphere");
            radius = atof(radiusAttribute);
            shape = new Sphere(radius);
        }
        else if (!strcmp(typeAttribute, "prism")) {
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
            while (tokenizer.hasMoreTokens()) {
                Coord point;
                point.x = atof(tokenizer.nextToken());
                if ((tok = tokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing prism y at %s", element->getSourceLocation());
                point.y = atof(tok);
                point.z = -height / 2;
                points.push_back(point);
            }
            if (points.size() < 3)
                throw cRuntimeError("prism needs at least three points at %s", element->getSourceLocation());
            convertPoints(points);
            shape = new Prism(height, Polygon(points));
        }
        else if (!strcmp(typeAttribute, "polyhedron")) {
            std::vector<Coord> points;
            const char *pointsAttribute = element->getAttribute("points");
            if (!pointsAttribute)
                throw cRuntimeError("Missing points attribute of polyhedron");
            cStringTokenizer tokenizer(pointsAttribute);
            while (tokenizer.hasMoreTokens()) {
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
            convertPoints(points);
            shape = new Polyhedron(points);
        }
        else
            throw cRuntimeError("Unknown shape type '%s'", typeAttribute);
        // insert
        idToShapeMap.insert(std::pair<int, const ShapeBase *>(id, shape));
    }
}

void PhysicalEnvironment::parseMaterials(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildrenByTagName("material");
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
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
        if (containsKey(idToMaterialMap, id))
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
    Coord computedSpaceMax = Coord::NIL;
    cXMLElementList children = xml->getChildren();
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
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
        // TODO what about geographic orientation? what about taking GeographicCoordinateSystem into account?
        Quaternion orientation;
        const char *orientationAttribute = element->getAttribute("orientation");
        if (orientationAttribute) {
            cStringTokenizer tokenizer(orientationAttribute);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation alpha at %s", element->getSourceLocation());
            auto alpha = deg(atof(tok));
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation beta at %s", element->getSourceLocation());
            auto beta = deg(atof(tok));
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing orientation gamma at %s", element->getSourceLocation());
            auto gamma = deg(atof(tok));
            orientation = Quaternion(EulerAngles(alpha, beta, gamma));
        }
        // shape
        Coord size = Coord::NIL;
        const ShapeBase *shape = nullptr;
        const char *shapeAttribute = element->getAttribute("shape");
        if (!shapeAttribute)
            throw cRuntimeError("Missing shape attribute of object");
        cStringTokenizer shapeTokenizer(shapeAttribute);
        const char *shapeType = shapeTokenizer.nextToken();
        if (!strcmp(shapeType, "cuboid")) {
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
        else if (!strcmp(shapeType, "sphere")) {
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing sphere radius at %s", element->getSourceLocation());
            double radius = atof(tok);
            shape = new Sphere(radius);
            size = Coord(radius, radius, radius) * 2;
            shapes.push_back(shape);
        }
        else if (!strcmp(shapeType, "prism")) {
            double height;
            if ((tok = shapeTokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing prism height at %s", element->getSourceLocation());
            height = atof(tok);
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens()) {
                Coord point;
                point.x = atof(shapeTokenizer.nextToken());
                if ((tok = shapeTokenizer.nextToken()) == nullptr)
                    throw cRuntimeError("Missing prism y at %s", element->getSourceLocation());
                point.y = atof(tok);
                point.z = -height / 2;
                points.push_back(point);
            }
            if (points.size() < 3)
                throw cRuntimeError("prism needs at least three points at %s", element->getSourceLocation());
            size = Box::computeBoundingBox(points).getSize();
            convertPoints(points);
            shape = new Prism(height, Polygon(points));
            shapes.push_back(shape);
        }
        else if (!strcmp(shapeType, "polyhedron")) {
            std::vector<Coord> points;
            while (shapeTokenizer.hasMoreTokens()) {
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
            size = Box::computeBoundingBox(points).getSize();
            convertPoints(points);
            shape = new Polyhedron(points);
            shapes.push_back(shape);
        }
        else {
            int id = atoi(shapeAttribute);
            shape = idToShapeMap[id];
        }
        if (!shape)
            throw cRuntimeError("Unknown shape '%s'", shapeAttribute);
        // position
        Coord position = Coord::NIL;
        const char *positionAttribute = element->getAttribute("position");
        if (positionAttribute) {
            cStringTokenizer tokenizer(positionAttribute);
            const char *kind = tokenizer.nextToken();
            if (!kind)
                throw cRuntimeError("Missing position kind");
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position x at %s", element->getSourceLocation());
            position.x = atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position y at %s", element->getSourceLocation());
            position.y = atof(tok);
            if ((tok = tokenizer.nextToken()) == nullptr)
                throw cRuntimeError("Missing position z at %s", element->getSourceLocation());
            position.z = atof(tok);
            if (!strcmp(kind, "min"))
                position += size / 2;
            else if (!strcmp(kind, "max"))
                position -= size / 2;
            else if (!strcmp(kind, "center"))
                position += Coord::ZERO;
            else
                throw cRuntimeError("Unknown position kind");
            if (coordinateSystem != nullptr) {
                auto convertedPosition = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(position.x), deg(position.y), m(0)));
                position.x = convertedPosition.x;
                position.y = convertedPosition.y;
            }
        }
        // material
        const Material *material;
        const char *materialAttribute = element->getAttribute("material");
        if (!materialAttribute)
            throw cRuntimeError("Missing material attribute of object");
        else if (containsKey(nameToMaterialMap, materialAttribute))
            material = nameToMaterialMap[materialAttribute];
        else if (MaterialRegistry::getInstance().getMaterial(materialAttribute))
            material = MaterialRegistry::getInstance().getMaterial(materialAttribute);
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
        if (lineColorAttribute) {
            if (strchr(lineColorAttribute, ' ')) {
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
                lineColor = cFigure::Color(lineColorAttribute);
        }
        // fill color
        cFigure::Color fillColor = cFigure::WHITE;
        const char *fillColorAttribute = element->getAttribute("fill-color");
        if (fillColorAttribute) {
            if (strchr(fillColorAttribute, ' ')) {
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
                fillColor = cFigure::Color(fillColorAttribute);
        }
        // opacity
        double opacity = 1;
        const char *opacityAttribute = element->getAttribute("opacity");
        if (opacityAttribute)
            opacity = atof(opacityAttribute);
        // texture
        const char *texture = element->getAttribute("texture");
        // tags
        const char *tags = element->getAttribute("tags");
        // insert object
        PhysicalObject *object = new PhysicalObject(name, id, position, orientation, shape, material, lineWidth, lineColor, fillColor, opacity, texture, tags);
        objects.push_back(object);
        if (id != -1)
            idToObjectMap.insert(std::pair<int, const PhysicalObject *>(id, object));
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

const PhysicalObject *PhysicalEnvironment::getObjectById(int id) const
{
    auto it = idToObjectMap.find(id);
    return (it != idToObjectMap.end()) ? it->second : nullptr;
}

void PhysicalEnvironment::visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const
{
    if (objectCache)
        objectCache->visitObjects(visitor, lineSegment);
    else
        for (const auto& elem : objects)
            visitor->visit(elem);
}

} // namespace physicalenvironment

} // namespace inet

