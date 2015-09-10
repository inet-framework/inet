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

#ifndef __INET_PHYSICALENVIRONMENT_H
#define __INET_PHYSICALENVIRONMENT_H

#include "inet/common/IVisitor.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/common/Rotation.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/environment/contract/IObjectCache.h"
#include "inet/environment/common/PhysicalObject.h"
#include "inet/environment/common/MaterialRegistry.h"

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
    class ObjectPositionComparator
    {
        protected:
            const Rotation &viewRotation;

        public:
            ObjectPositionComparator(const Rotation &viewRotation) : viewRotation(viewRotation) {}

            bool operator() (const PhysicalObject *left, const PhysicalObject *right) const
            {
                return viewRotation.rotateVectorClockwise(left->getPosition()).z < viewRotation.rotateVectorClockwise(right->getPosition()).z;
            }
    };

  protected:
    /** @name Parameters */
    //@{
    K temperature;
    Coord spaceMin;
    Coord spaceMax;
    double axisLength;
    //@}

    /** @name Internal state */
    //@{
    EulerAngles viewAngle;
    Rotation viewRotation;
    cFigure::Point viewTranslation;
    std::vector<const ShapeBase *> shapes;
    std::vector<const Material *> materials;
    std::vector<const PhysicalObject *> objects;
    //@}

    /** @name Cache */
    //@{
    IObjectCache *objectCache;
    std::map<int, const ShapeBase *> idToShapeMap;  // shared shapes
    std::map<int, const Material *> idToMaterialMap;
    std::map<int, const PhysicalObject *> idToObjectMap;
    std::map<const std::string, const Material *> nameToMaterialMap;
    //@}

    /** @name Graphics */
    //@{
    cGroupFigure *objectsLayer;
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);

    virtual void updateCanvas();

    virtual void computeFacePoints(const PhysicalObject *object, std::vector<std::vector<Coord> >& faces, const Rotation& rotation);
    virtual EulerAngles computeViewAngle(const char *viewAngle);
    virtual cFigure::Point computeViewTranslation(const char *viewTranslation);

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    virtual K getTemperature() const { return temperature; }
    virtual const Coord& getSpaceMin() const override { return spaceMin; }
    virtual const Coord& getSpaceMax() const override { return spaceMax; }

    virtual const EulerAngles& getViewAngle() const override { return viewAngle; }
    virtual const Rotation& getViewRotation() const override { return viewRotation; }
    virtual const cFigure::Point& getViewTranslation() const override { return viewTranslation; }
    virtual const IMaterialRegistry *getMaterialRegistry() const override { return &MaterialRegistry::singleton; }

    virtual int getNumObjects() const { return objects.size(); }
    virtual const PhysicalObject *getObject(int index) const { return objects[index]; }
    virtual const PhysicalObject *getObjectById(int id) const;

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENT_H

