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
        parseObjects(par("objects"));
    }
    else if (stage == INITSTAGE_LAST) {
        updateCanvas();
    }
}

void PhysicalEnvironment::parseObjects(cXMLElement *xml)
{
    std::string rootTag = xml->getTagName();
    cXMLElementList list = xml->getChildren();
    // TODO: move parsers to the appropriate classes
    for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
        cXMLElement *e = *i;
        std::string tag = e->getTagName();
        // id
        const char *idAttribute = e->getAttribute("id");
        int id = -1;
        if (idAttribute)
            id = atoi(idAttribute);
        // position
        Coord position;
        const char *positionAttribute = e->getAttribute("position");
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
        const char *orientationAttribute = e->getAttribute("orientation");
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
        Shape *shape;
        const char *shapeAttribute = e->getAttribute("shape");
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
        else
            throw cRuntimeError("Unknown shape '%s'", shapeAttribute);
        // material
        Material *material;
        const char *materialAttribute = e->getAttribute("material");
        if (!strcmp(materialAttribute, "brick"))
            material = &Material::brick;
        else if (!strcmp(materialAttribute, "concrete"))
            material = &Material::concrete;
        else
            throw cRuntimeError("Unknown material '%s'", materialAttribute);
        // color
#ifdef __CCANVAS_H
        cFigure::Color color;
        const char *colorAttribute = e->getAttribute("color");
        if (colorAttribute) {
            cStringTokenizer tokenizer(colorAttribute);
            if (tokenizer.hasMoreTokens())
                color.red = atoi(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                color.green = atoi(tokenizer.nextToken());
            if (tokenizer.hasMoreTokens())
                color.blue = atoi(tokenizer.nextToken());
        }
        PhysicalObject *object = new PhysicalObject(id, position, orientation, shape, material, color);
#else // ifdef __CCANVAS_H
        PhysicalObject *object = new PhysicalObject(id, position, orientation, shape, material);
#endif // ifdef __CCANVAS_H
        objects.push_back(object);
    }
}

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
            figure->setP1(cFigure::Point(position.x - size.x / 2, position.y - size.y / 2));
            figure->setP2(cFigure::Point(position.x + size.x / 2, position.y + size.y / 2));
            figure->setFillColor(object->getColor());
            layer->addChild(figure);
            continue;
        }
        const Sphere *sphere = dynamic_cast<const Sphere *>(shape);
        if (sphere) {
            double radius = sphere->getRadius();
            cOvalFigure *figure = new cOvalFigure(NULL);
            figure->setFilled(true);
            figure->setP1(cFigure::Point(position.x - radius, position.y - radius));
            figure->setP2(cFigure::Point(position.x + radius, position.y + radius));
            figure->setFillColor(object->getColor());
            layer->addChild(figure);
            continue;
        }
        throw cRuntimeError("Unknown shape");
    }
#endif // ifdef __CCANVAS_H
}

} // namespace inet

