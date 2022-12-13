//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MATERIALREGISTRY_H
#define __INET_MATERIALREGISTRY_H

#include <map>

#include "inet/environment/contract/IMaterialRegistry.h"

namespace inet {

namespace physicalenvironment {

class INET_API MaterialRegistry : public IMaterialRegistry
{
  protected:
    mutable std::map<const std::string, const Material *> materials;

  protected:
    void addMaterial(const Material *material) const;

  public:
    MaterialRegistry();
    virtual ~MaterialRegistry();

    virtual const Material *getMaterial(const char *name) const override;

    static MaterialRegistry& getInstance();
};

} // namespace physicalenvironment

} // namespace inet

#endif

