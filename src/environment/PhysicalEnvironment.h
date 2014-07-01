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

namespace inet {

/**
 * This class represents the physical environment specifying certain physical properties.
 */
// TODO: add loading objects from XML file
class INET_API PhysicalEnvironment : public cModule
{
  protected:
    K temperature;
    Pa pressure;
    percent relativeHumidity;
    Coord spaceMin;
    Coord spaceMax;
    const char *viewAngle;
    std::map<int, const Shape *> shapes;
    std::map<int, const Material *> materials;
    std::vector<PhysicalObject *> objects;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

    virtual cFigure::Point projectPoint(Coord point);
    virtual void parseShapes(cXMLElement *xml);
    virtual void parseMaterials(cXMLElement *xml);
    virtual void parseObjects(cXMLElement *xml);
    virtual void updateCanvas();

  public:
    PhysicalEnvironment();
    virtual ~PhysicalEnvironment();

    virtual K getTemperature() const { return temperature; }
    virtual Pa getPressure() const { return pressure; }
    virtual percent getRelativeHumidity() const { return relativeHumidity; }
    virtual const Coord getSpaceMin() { return spaceMin; }
    virtual const Coord getSpaceMax() { return spaceMax; }
    virtual const std::vector<PhysicalObject *>& getObjects() { return objects; }
};

} // namespace inet

#endif // ifndef __INET_PHYSICALENVIRONMENT_H

