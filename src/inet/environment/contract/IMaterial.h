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

#ifndef __INET_IMATERIAL_H
#define __INET_IMATERIAL_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"

namespace inet {

namespace physicalenvironment {

using namespace units::values;
using namespace units::constants;

class INET_API IMaterial
{
  public:
    virtual Ohmm getResistivity() const = 0;
    virtual double getRelativePermittivity() const = 0;
    virtual double getRelativePermeability() const = 0;
    virtual double getDielectricLossTangent(Hz frequency) const = 0;
    virtual double getRefractiveIndex() const = 0;
    virtual mps getPropagationSpeed() const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif // ifndef __INET_IMATERIAL_H

