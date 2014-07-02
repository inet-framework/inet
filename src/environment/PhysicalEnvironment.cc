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

#include "PhysicalEnvironment.h"
#include "Cuboid.h"
#include "Sphere.h"
#include "Material.h"

namespace inet {

Define_Module(PhysicalEnvironment);

PhysicalEnvironment::PhysicalEnvironment() :
    temperature(sNaN),
    pressure(sNaN),
    relativeHumidity(sNaN),
    spaceMin(Coord(sNaN, sNaN, sNaN)),
    spaceMax(Coord(sNaN, sNaN, sNaN))
{
}

PhysicalEnvironment::~PhysicalEnvironment()
{
    for (std::map<int, const Shape *>::iterator it = shapes.begin(); it != shapes.end(); it++)
        delete it->second;
    for (std::map<int, const Material *>::iterator it =  materials.begin(); it != materials.end(); it++)
        delete it->second;
    for (std::vector<PhysicalObject *>::iterator it = objects.begin(); it != objects.end(); it++)
        delete *it;
}

void PhysicalEnvironment::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        temperature = K(par("temperature"));
        pressure = Pa(par("pressure"));
        relativeHumidity = percent(par("relativeHumidity"));
        spaceMin.x = par("spaceMinX");
        spaceMin.y = par("spaceMinY");
        spaceMin.z = par("spaceMinZ");
        spaceMax.x = par("spaceMaxX");
        spaceMax.y = par("spaceMaxY");
        spaceMax.z = par("spaceMaxZ");
        viewAngle = par("viewAngle");
        cXMLElement *environment = par("environment");
        parseShapes(environment);
        parseMaterials(environment);
        parseObjects(environment);
    }
    else if (stage == INITSTAGE_LAST) {
        updateCanvas();
    }
}

void PhysicalEnvironment::parseShapes(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildren();
    // TODO: move parsers to the appropriate classes
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
        cXMLElement *element = *it;
        const char *tag = element->getTagName();
        if (strcmp(tag, "shape"))
            continue;
        Shape *shape = NULL;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // type
        const char *typeAttribute = element->getAttribute("type");
        if (!typeAttribute)
            throw cRuntimeError("Missing type attribute of shape");
        else if (!strcmp(typeAttribute, "cuboid")) {
            Coord size;
            const char *sizeAttribute = element->getAttribute("size");
            if (!sizeAttribute)
                throw cRuntimeError("Missing size attribute of cuboid shape");
            cStringTokenizer tokenizer(sizeAttribute);
            if (tokenizer.hasMoreTokens())
                size.x = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                size.y = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                size.z = atof(tokenizer.nextToken());
            shape = new Cuboid(size);
        }
        else if (!strcmp(typeAttribute, "sphere")) {
            double radius;
            const char *radiusAttribute = element->getAttribute("radius");
            if (!radiusAttribute)
                throw cRuntimeError("Missing radius attribute of sphere shape");
            cStringTokenizer tokenizer(radiusAttribute);
            if (tokenizer.hasMoreTokens())
                radius = atof(tokenizer.nextToken());
            shape = new Sphere(radius);
        }
        // insert
        shapes.insert(std::pair<int, Shape *>(id, shape));
    }
}

void PhysicalEnvironment::parseMaterials(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildren();
    // TODO: move parsers to the appropriate classes
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
        cXMLElement *element = *it;
        const char *tag = element->getTagName();
        if (strcmp(tag, "material"))
            continue;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // resistivity
        Ohmm resistivity = Ohmm(NaN);
        const char *resistivityAttribute = element->getAttribute("resistivity");
        if (resistivityAttribute)
            resistivity = Ohmm(atof(resistivityAttribute));
        // relativePermittivity
        double relativePermittivity = NaN;
        const char *relativePermittivityAttribute = element->getAttribute("relativePermittivity");
        if (relativePermittivity)
            relativePermittivity = atof(relativePermittivityAttribute);
        // relativePermeability
        double relativePermeability = NaN;
        const char *relativePermeabilityAttribute = element->getAttribute("relativePermeability");
        if (relativePermeabilityAttribute)
            relativePermeability = atof(relativePermeabilityAttribute);
        // insert
        Material *material = new Material(resistivity, relativePermittivity, relativePermeability);
        materials.insert(std::pair<int, Material *>(id, material));
    }
}

void PhysicalEnvironment::parseObjects(cXMLElement *xml)
{
    cXMLElementList children = xml->getChildren();
    // TODO: move parsers to the appropriate classes
    for (cXMLElementList::const_iterator it = children.begin(); it != children.end(); ++it) {
        cXMLElement *element = *it;
        const char *tag = element->getTagName();
        if (strcmp(tag, "object"))
            continue;
        // id
        const char *idAttribute = element->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // position
        Coord position;
        const char *positionAttribute = element->getAttribute("position");
        if (positionAttribute) {
            cStringTokenizer tokenizer(positionAttribute);
            if (tokenizer.hasMoreTokens())
                position.x = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                position.y = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                position.z = atof(tokenizer.nextToken());
        }
        // orientation
        EulerAngles orientation;
        const char *orientationAttribute = element->getAttribute("orientation");
        if (orientationAttribute) {
            cStringTokenizer tokenizer(orientationAttribute);
            if (tokenizer.hasMoreTokens())
                orientation.alpha = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                orientation.beta = atof(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                orientation.gamma = atof(tokenizer.nextToken());
        }
        // shape
        const Shape *shape;
        const char *shapeAttribute = element->getAttribute("shape");
        if (!shapeAttribute)
            throw cRuntimeError("Missing shape attribute of object");
        cStringTokenizer shapeTokenizer(shapeAttribute);
        const char *shapeType = shapeTokenizer.nextToken();
        if (!strcmp(shapeType, "cuboid")) {
            Coord size;
            if (shapeTokenizer.hasMoreTokens())
                size.x = atof(shapeTokenizer.nextToken());
            if (shapeTokenizer.hasMoreTokens())
                size.y = atof(shapeTokenizer.nextToken());
            if (shapeTokenizer.hasMoreTokens())
                size.z = atof(shapeTokenizer.nextToken());
            shape = new Cuboid(size);
        }
        else if (!strcmp(shapeType, "sphere")) {
            double radius = 0;
            if (shapeTokenizer.hasMoreTokens())
                radius = atof(shapeTokenizer.nextToken());
            shape = new Sphere(radius);
        }
        else {
            int id = atoi(shapeAttribute);
            shape = shapes[id];
        }
        if (!shape)
            throw cRuntimeError("Unknown shape '%s'", shapeAttribute);
        // material
        const Material *material;
        const char *materialAttribute = element->getAttribute("material");
        if (!materialAttribute)
            throw cRuntimeError("Missing material attribute of object");
        else if (!strcmp(materialAttribute, "brick"))
            material = &Material::brick;
        else if (!strcmp(materialAttribute, "concrete"))
            material = &Material::concrete;
        else {
            int id = atoi(materialAttribute);
            material = materials[id];
        }
        if (!material)
            throw cRuntimeError("Unknown material '%s'", materialAttribute);
        // color
#ifdef __CCANVAS_H
        cFigure::Color color;
        const char *colorAttribute = element->getAttribute("color");
        if (colorAttribute) {
            cStringTokenizer tokenizer(colorAttribute);
            if (tokenizer.hasMoreTokens())
                color.red = atoi(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                color.green = atoi(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                color.blue = atoi(tokenizer.nextToken());
        }
        // insert
        PhysicalObject *object = new PhysicalObject(id, position, orientation, shape, material, color);
#else // ifdef __CCANVAS_H
        PhysicalObject *object = new PhysicalObject(id, position, orientation, shape, material);
#endif // ifdef __CCANVAS_H
        objects.push_back(object);
    }
}

#ifdef __CCANVAS_H
cFigure::Point PhysicalEnvironment::projectPoint(Coord point)
{
    if (!strcmp(viewAngle, "x"))
        return cFigure::Point(point.y, point.z);
    else if (!strcmp(viewAngle, "y"))
        return cFigure::Point(point.x, point.z);
    else if (!strcmp(viewAngle, "z"))
        return cFigure::Point(point.x, point.y);
    else
        throw cRuntimeError("Unknown view angle");
}
#endif // ifdef __CCANVAS_H

void PhysicalEnvironment::updateCanvas()
{
#ifdef __CCANVAS_H
    cCanvas *canvas = getParentModule()->getCanvas();
    cLayer *layer = canvas->getDefaultLayer();
    for (std::vector<PhysicalObject *>::iterator it = objects.begin(); it != objects.end(); it++) {
        PhysicalObject *object = *it;
        const Shape *shape = object->getShape();
        const Coord& position = object->getPosition();
        const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
        if (cuboid) {
            const Coord& size = cuboid->getSize();
            cRectangleFigure *figure = new cRectangleFigure(NULL);
            figure->setFilled(true);
            figure->setP1(projectPoint(position - size / 2));
            figure->setP2(projectPoint(position + size / 2));
            figure->setFillColor(object->getColor());
            layer->addChild(figure);
            continue;
        }
        const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
        if (sphere) {
            double radius = sphere->getRadius();
            cOvalFigure *figure = new cOvalFigure(NULL);
            figure->setFilled(true);
            figure->setP1(projectPoint(position - Coord(radius, radius, radius)));
            figure->setP2(projectPoint(position + Coord(radius, radius, radius)));
            figure->setFillColor(object->getColor());
            layer->addChild(figure);
            continue;
        }
        throw cRuntimeError("Unknown shape");
    }
#endif // ifdef __CCANVAS_H
}

} // namespace inet

