//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMATERIALREGISTRY_H
#define __INET_IMATERIALREGISTRY_H

#include "inet/environment/common/Material.h"

namespace inet {

namespace physicalenvironment {

class INET_API IMaterialRegistry
{
  public:
    virtual const Material *getMaterial(const char *name) const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif

