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

#include "inet/common/INETMath.h"
#include "inet/environment/common/Material.h"

namespace inet {

namespace physicalenvironment {

Material::Material(const char *name, Ohmm resistivity, double relativePermittivity, double relativePermeability) :
    cNamedObject(name, false),
    resistivity(resistivity),
    relativePermittivity(relativePermittivity),
    relativePermeability(relativePermeability)
{
}

double Material::getDielectricLossTangent(Hz frequency) const
{
    return unit(1.0 / (2 * M_PI * frequency * resistivity * relativePermittivity * e0)).get();
}

double Material::getRefractiveIndex() const
{
    return std::sqrt(relativePermittivity * relativePermeability);
}

mps Material::getPropagationSpeed() const
{
    return mps(SPEED_OF_LIGHT) / getRefractiveIndex();
}

} // namespace physicalenvironment

} // namespace inet

