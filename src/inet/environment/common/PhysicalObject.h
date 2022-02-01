//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALOBJECT_H
#define __INET_PHYSICALOBJECT_H

#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/common/Material.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

namespace physicalenvironment {

/**
 * This class represents an immobile physical object, a rigid body and its
 * physical properties. The properties of physical objects cannot change over
 * time.
 */
class INET_API PhysicalObject : public cNamedObject, public IPhysicalObject
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
    Quaternion orientation;
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
    const char *texture;
    const char *tags;
    //@}

  public:
    PhysicalObject(const char *name, int id, const Coord& position, const Quaternion& orientation, const ShapeBase *shape, const Material *material, double lineWidth, const cFigure::Color& lineColor, const cFigure::Color& fillColor, double opacity, const char *texture, const char *tags);

    virtual int getId() const { return id; }

    virtual const Coord& getPosition() const override { return position; }
    virtual const Quaternion& getOrientation() const override { return orientation; }

    virtual const ShapeBase *getShape() const override { return shape; }
    virtual const Material *getMaterial() const override { return material; }

    virtual double getLineWidth() const override { return lineWidth; }
    virtual const cFigure::Color& getLineColor() const override { return lineColor; }
    virtual const cFigure::Color& getFillColor() const override { return fillColor; }
    virtual double getOpacity() const override { return opacity; }
    virtual const char *getTexture() const override { return texture; }
    virtual const char *getTags() const override { return tags; }
};

} // namespace physicalenvironment

} // namespace inet

#endif

