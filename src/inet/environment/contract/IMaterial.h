//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMATERIAL_H
#define __INET_IMATERIAL_H

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

#endif

