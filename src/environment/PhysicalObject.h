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

#include "Shape3D.h"
#include "EulerAngles.h"
#include "Material.h"

namespace inet {

/**
 * This class represents a physical object, a rigid body and its physical properties.
 * The object can change its position and orientation over time.
 */
class INET_API PhysicalObject : public cNamedObject
{
  protected:
    /** Globally unique identifier for the whole lifetime of the simulation among all objects. */
    const int id;

    /** @name Object properties */
    //@{
    /** The center of the object's bounding box. */
    Coord position;
    /** The orientation of the object relative to the default orientation of the shape. */
    EulerAngles orientation; // TODO: (sequence of rotation axes: xyz or something else?)
    /** The shape of the object independently of its position and orientation. */
    const Shape3D *shape;
    /** The material of the object determines its physical properties. */
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

    /** @name Cache */
    //@{
    cFigure *figure;
    //@}

  public:
    PhysicalObject(const char *name, int id, const Coord& position, const EulerAngles& orientation, const Shape3D *shape, const Material *material, double lineWidth, const cFigure::Color& lineColor, const cFigure::Color& fillColor, double opacity, const char *tags);
    virtual ~PhysicalObject();

    virtual int getId() const { return id; }

    virtual const Coord& getPosition() const { return position; }
    virtual const EulerAngles& getOrientation() const { return orientation; }

    virtual const Shape3D *getShape() const { return shape; }
    virtual const Material *getMaterial() const { return material; }

    virtual double getLineWidth() const { return lineWidth; }
    virtual const cFigure::Color& getLineColor() const { return lineColor; }
    virtual const cFigure::Color& getFillColor() const { return fillColor; }
    virtual double getOpacity() const { return opacity; }
    virtual const char *getTags() const { return tags; }
};

} // namespace inet

#endif // ifndef __INET_PHYSICALOBJECT_H

