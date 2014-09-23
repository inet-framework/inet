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

#include "inet/environment/cache/IObjectCache.h"
#include "inet/environment/PhysicalObject.h"
#include "inet/common/IVisitor.h"
#include "inet/common/geometry/base/ShapeBase.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/common/Rotation.h"

namespace inet {

/**
 * This class represents the physical environment. The physical environment
 * contains a set of physical objects and it also specifies certain physical
 * properties.
 *
 * The physical environment draws the physical objects on the canvas of its
 * parent module.
 */
class INET_API PhysicalEnvironment : public cModule
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
    //@}

    /** @name Internal state */
    //@{
    EulerAngles viewAngle;
    Rotation viewRotation;
    std::vector<const ShapeBase *> shapes;
    std::vector<const Material *> materials;
    std::vector<const PhysicalObject *> objects;
    //@}

    /** @name Cache */
    //@{
    IObjectCache *objectCache;
    std::map<int, const ShapeBase *> idToShapeMap;
    std::map<int, const Material *> idToMaterialMap;
    std::map<int, const PhysicalObject *> idToObjectMap;
    std::map<const std::string, const Material *> nameToMaterialMap;
    //@}

    /** @name Graphics */
    //@{
    cGroupFigure *objectsLayer;
    //@}

  protected:
    static cFigure::Point computeCanvasPoint(const Coord& point, const Rotation& rotation);

    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleParameterChange(const char *name);

    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);

    virtual void updateCanvas();

    virtual void computeFacePoints(const PhysicalObject *object, std::vector<std::vector<Coord> >& faces, const Rotation& rotation);
    virtual EulerAngles computeViewAngle(const char *viewAngle);

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    // TODO: eventually delete this function?
    static cFigure::Point computeCanvasPoint(Coord point);

    virtual K getTemperature() const { return temperature; }
    virtual const Coord& getSpaceMin() const { return spaceMin; }
    virtual const Coord& getSpaceMax() const { return spaceMax; }

    virtual const EulerAngles& getViewAngle() const { return viewAngle; }
    virtual const Rotation& getViewRotation() const { return viewRotation; }

    virtual int getNumObjects() const { return objects.size(); }
    virtual const PhysicalObject *getObject(int index) const { return objects[index]; }
    virtual const PhysicalObject *getObjectById(int id) const;

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const;
};

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENT_H

