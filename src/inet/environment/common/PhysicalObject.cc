//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/common/PhysicalObject.h"

namespace inet {

namespace physicalenvironment {

PhysicalObject::PhysicalObject(const char *name, int id, const Coord& position, const Quaternion& orientation, const ShapeBase *shape, const Material *material, double lineWidth, const cFigure::Color& lineColor, const cFigure::Color& fillColor, double opacity, const char *texture, const char *tags) :
    cNamedObject(name),
    id(id),
    position(position),
    orientation(orientation),
    shape(shape),
    material(material),
    lineWidth(lineWidth),
    lineColor(lineColor),
    fillColor(fillColor),
    opacity(opacity),
    texture(texture),
    tags(tags)
{
}

} // namespace physicalenvironment

} // namespace inet

