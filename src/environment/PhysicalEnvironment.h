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

#ifndef __INET_PHYSICALENVIRONMENT_H_
#define __INET_PHYSICALENVIRONMENT_H_

#include "PhysicalObject.h"

class INET_API PhysicalEnvironment : public cSimpleModule
{
    protected:
        K temperature;
        Pa pressure;
        percent relativeHumidity;
        std::vector<PhysicalObject *> objects;

    public:
        PhysicalEnvironment();

        virtual K getTemperature() const { return temperature; }
        virtual Pa getPressure() const { return pressure; }
        virtual percent getRelativeHumidity() const { return relativeHumidity; }
        virtual const std::vector<PhysicalObject *> &getObjects() { return objects; }
};

#endif
