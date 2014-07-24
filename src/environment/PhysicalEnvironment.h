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

#include "PhysicalObject.h"
#include "IVisitor.h"
#include "LineSegment.h"
#include "Box.h"

namespace inet {

/**
 * This class represents the physical environment specifying certain physical properties.
 */
class INET_API PhysicalEnvironment : public cModule
{
  public:
    class IObjectCache
    {
        public:
            virtual bool insertObject(const PhysicalObject *object) = 0;
            virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const = 0;
            virtual void buildCache() = 0;
    };
  protected:
    K temperature;
    Pa pressure;
    percent relativeHumidity;
    Coord spaceMin;
    Coord spaceMax;
    const char *viewAngle;
    std::map<int, const Shape3D *> shapes;
    std::map<int, const Material *> materials;
    IObjectCache *objectCache;
    std::vector<PhysicalObject *> objects;

    cGroupFigure *objectsLayer;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

    static cFigure::Point computeCanvasPoint(Coord point, char viewAngle);

    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);
    virtual void updateCanvas();
    Box calculateBoundingBox(const std::vector<Coord>& points) const;

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    static cFigure::Point computeCanvasPoint(Coord point);

    virtual bool hasObjectCache() const { return objectCache != NULL; }
    virtual K getTemperature() const { return temperature; }
    virtual Pa getPressure() const { return pressure; }
    virtual percent getRelativeHumidity() const { return relativeHumidity; }
    virtual const Coord getSpaceMin() const { return spaceMin; }
    virtual const Coord getSpaceMax() const { return spaceMax; }
    virtual const std::vector<PhysicalObject *>& getObjects() const;
    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const;
};

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENT_H

