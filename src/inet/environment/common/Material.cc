//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/environment/common/Material.h"

#include "inet/common/INETMath.h"

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

