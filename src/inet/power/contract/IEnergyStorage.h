//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYSTORAGE_H
#define __INET_IENERGYSTORAGE_H

#include "inet/power/contract/IEnergySink.h"
#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy storage
 * models to integrate with other parts of the power model. This interface is
 * extended by various energy storage interfaces. Actual energy storage
 * implementations should implement one of the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergyStorage : public virtual IEnergySource, public virtual IEnergySink
{
};

} // namespace power

} // namespace inet

#endif

