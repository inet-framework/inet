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

#ifndef __INET_PHYSICALOBJECT_H
#define __INET_PHYSICALOBJECT_H

#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/environment/Material.h"

namespace inet {

/**
 * This class represents an immobile physical object, a rigid body and its
 * physical properties. The properties of physical objects cannot change over
 * time.
 */
class INET_API PhysicalObject : public cNamedObject
{
  protected:
    /**
     * A globally unique identifier for the whole lifetime of the simulation
     * among all physical objects.
     */
    const int id;

    /** @name Object properties */
    //@{
    /**
     * The center of the object's bounding box.
     */
    Coord position;
    /**
     * The orientation of the object relative to the default orientation of the shape.
     */
    EulerAngles orientation;
    /**
     * The shape of the object independently of its position and orientation.
     * The physical object doesn't own its shape.
     */
    const ShapeBase *shape;
    /**
     * The material of the object determines its physical properties.
     * The physical object doesn't own its material.
     */
    const Material *material;
    //@}

    /** @name Graphics properties */
    //@{
    const double lineWidth;
    const cFigure::Color lineColor;
    const cFigure::Color fillColor;
    const double opacity;
    const char *tags;
    //@}

  public:
    PhysicalObject(const char *name, int id, const Coord& position, const EulerAngles& orientation, const ShapeBase *shape, const Material *material, double lineWidth, const cFigure::Color& lineColor, const cFigure::Color& fillColor, double opacity, const char *tags);

    virtual int getId() const { return id; }

    virtual const Coord& getPosition() const { return position; }
    virtual const EulerAngles& getOrientation() const { return orientation; }

    virtual const ShapeBase *getShape() const { return shape; }
    virtual const Material *getMaterial() const { return material; }

    virtual double getLineWidth() const { return lineWidth; }
    virtual const cFigure::Color& getLineColor() const { return lineColor; }
    virtual const cFigure::Color& getFillColor() const { return fillColor; }
    virtual double getOpacity() const { return opacity; }
    virtual const char *getTags() const { return tags; }
};

} // namespace inet

#endif // ifndef __INET_PHYSICALOBJECT_H

