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

#include "IObjectCache.h"
#include "IVisitor.h"
#include "PhysicalObject.h"
#include "LineSegment.h"
#include "Rotation.h"

namespace inet {

/**
 * This class represents the physical environment specifying certain physical properties.
 */
class INET_API PhysicalEnvironment : public cModule
{
  public:
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
    K temperature;
    Pa pressure;
    percent relativeHumidity;
    Coord spaceMin;
    Coord spaceMax;
    EulerAngles viewAngle;
    Rotation viewRotation;
    std::map<int, const Shape3D *> shapes;
    std::map<int, const Material *> materials;
    std::vector<PhysicalObject *> objects;
    IObjectCache *objectCache;
    cGroupFigure *objectsLayer;

  protected:
    static cFigure::Point computeCanvasPoint(const Coord& point, const Rotation& rotation);

    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleParameterChange(const char *name);

    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);
    virtual void updateCanvas();
    virtual void computeFacePoints(PhysicalObject *object, std::vector<std::vector<Coord> >& faces, const Rotation& rotation);
    virtual EulerAngles computeViewAngle(const char *viewAngle);

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    // TODO: eventually delete this function?
    static cFigure::Point computeCanvasPoint(Coord point);

    virtual K getTemperature() const { return temperature; }
    virtual Pa getPressure() const { return pressure; }
    virtual percent getRelativeHumidity() const { return relativeHumidity; }
    virtual const Coord& getSpaceMin() const { return spaceMin; }
    virtual const Coord& getSpaceMax() const { return spaceMax; }
    virtual const EulerAngles& getViewAngle() const { return viewAngle; }
    virtual const Rotation& getViewRotation() const { return viewRotation; }
    virtual const std::vector<PhysicalObject *>& getObjects() const;
    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const;
};

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENT_H

