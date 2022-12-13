//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PHYSICALENVIRONMENT_H
#define __INET_PHYSICALENVIRONMENT_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/environment/common/MaterialRegistry.h"
#include "inet/environment/common/PhysicalObject.h"
#include "inet/environment/contract/IObjectCache.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"

namespace inet {

namespace physicalenvironment {

/**
 * This class represents the physical environment. The physical environment
 * contains a set of physical objects and it also specifies certain physical
 * properties.
 *
 * The physical environment draws the physical objects on the canvas of its
 * parent module.
 */
class INET_API PhysicalEnvironment : public cModule, public IPhysicalEnvironment
{
  protected:
    /** @name Parameters */
    //@{
    IGeographicCoordinateSystem *coordinateSystem = nullptr;
    K temperature;
    Coord spaceMin;
    Coord spaceMax;
    //@}

    /** @name Submodules */
    //@{
    IObjectCache *objectCache = nullptr;
    IGround *ground = nullptr;
    //@}

    /** @name Internal state */
    //@{
    std::vector<const ShapeBase *> shapes;
    std::vector<const Material *> materials;
    std::vector<const PhysicalObject *> objects;
    //@}

    /** @name Cache */
    //@{
    std::map<int, const ShapeBase *> idToShapeMap; // shared shapes
    std::map<int, const Material *> idToMaterialMap;
    std::map<int, const PhysicalObject *> idToObjectMap;
    std::map<std::string, const Material *> nameToMaterialMap;
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual void convertPoints(std::vector<Coord>& points);
    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    virtual IObjectCache *getObjectCache() const override { return objectCache; }
    virtual IGround *getGround() const override { return ground; }

    virtual K getTemperature() const { return temperature; }
    virtual const Coord& getSpaceMin() const override { return spaceMin; }
    virtual const Coord& getSpaceMax() const override { return spaceMax; }

    virtual const IMaterialRegistry *getMaterialRegistry() const override { return &MaterialRegistry::getInstance(); }

    virtual int getNumObjects() const override { return objects.size(); }
    virtual const PhysicalObject *getObject(int index) const override { return objects[index]; }
    virtual const PhysicalObject *getObjectById(int id) const override;

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif

