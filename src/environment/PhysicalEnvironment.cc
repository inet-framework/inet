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
#include "Material.h"

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
    if (stage == INITSTAGE_LOCAL)
    {
        temperature = K(par("temperature"));
        pressure = Pa(par("pressure"));
        relativeHumidity = percent(par("relativeHumidity"));
        spaceMin.x = par("spaceMinX");
        spaceMin.y = par("spaceMinY");
        spaceMin.z = par("spaceMinZ");
        spaceMax.x = par("spaceMaxX");
        spaceMax.y = par("spaceMaxY");
        spaceMax.z = par("spaceMaxZ");
    }
    else if (stage == INITSTAGE_LAST)
    {
        createRandomObjects();
        updateCanvas();
    }
}

void PhysicalEnvironment::createRandomObjects()
{
    for (int i = 0; i < 10; i++)
    {
        Coord size(uniform(10, 100), uniform(10, 100), 0);
        Coord min;
        min.x = uniform(spaceMin.x, spaceMax.x - size.x);
        min.y = uniform(spaceMin.y, spaceMax.y - size.y);
        min.z = uniform(spaceMin.z, spaceMax.z - size.z);
        Coord max = min + size;
        PhysicalObject *object = new PhysicalObject(new Cuboid(min, max), &Material::concrete);
        objects.push_back(object);
    }
//    PhysicalObject *object = new PhysicalObject(new Cuboid(Coord(0, 200, 0), Coord(600, 210, 0)), &Material::concrete);
//    objects.push_back(object);
}

void PhysicalEnvironment::updateCanvas()
{
    cCanvas *canvas = getParentModule()->getCanvas();
    cLayer *layer = canvas->getDefaultLayer();
    for (std::vector<PhysicalObject *>::iterator it = objects.begin(); it != objects.end(); it++)
    {
        PhysicalObject *object = *it;
        const Shape *shape = object->getShape();
        const Cuboid *cuboid = dynamic_cast<const Cuboid *>(shape);
        if (cuboid)
        {
            const Coord& min = cuboid->getMin();
            const Coord& max = cuboid->getMax();
            cRectangleFigure *figure = new cRectangleFigure(NULL);
            figure->setFilled(true);
            figure->setP1(cFigure::Point(min.x, min.y));
            figure->setP2(cFigure::Point(max.x, max.y));
            layer->addChild(figure);
        }
        else
            throw cRuntimeError("Unknown shape");
    }
}
