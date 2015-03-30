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

#ifndef __INET_MATERIAL_H
#define __INET_MATERIAL_H

#include "inet/environment/contract/IMaterial.h"

namespace inet {

namespace physicalenvironment {

/**
 * This class represents a material with its physical properties.
 */
// TODO: what about the dependency of physical properties on temperature, pressure, frequency, etc.?
class INET_API Material : public cNamedObject, public IMaterial
{
  protected:
    const Ohmm resistivity;
    const double relativePermittivity;
    const double relativePermeability;

  public:
    Material(const char *name, Ohmm resistivity, double relativePermittivity, double relativePermeability);

    virtual Ohmm getResistivity() const override { return resistivity; }
    virtual double getRelativePermittivity() const override { return relativePermittivity; }
    virtual double getRelativePermeability() const override { return relativePermeability; }
    virtual double getDielectricLossTangent(Hz frequency) const override;
    virtual double getRefractiveIndex() const override;
    virtual mps getPropagationSpeed() const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif // ifndef __INET_MATERIAL_H

