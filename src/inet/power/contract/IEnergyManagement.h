//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYMANAGEMENT_H
#define __INET_IENERGYMANAGEMENT_H

#include "inet/power/contract/IEnergyStorage.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy management
 * models to integrate with other parts of the power model. This interface is
 * extended by various energy storage interfaces. Actual energy storage
 * implementations should implement one of the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergyManagement
{
  public:
    virtual ~IEnergyManagement() {}

    /**
     * Returns the energy storage that is managed by this energy management.
     * This function never returns nullptr.
     */
    virtual IEnergyStorage *getEnergyStorage() const = 0;
};

} // namespace power

} // namespace inet

#endif

