//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MATERIAL_H
#define __INET_MATERIAL_H

#include "inet/environment/contract/IMaterial.h"

namespace inet {

namespace physicalenvironment {

/**
 * This class represents a material with its physical properties.
 */
// TODO what about the dependency of physical properties on temperature, pressure, frequency, etc.?
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

#endif

