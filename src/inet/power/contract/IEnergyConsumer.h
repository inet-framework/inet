//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IENERGYCONSUMER_H
#define __INET_IENERGYCONSUMER_H

#include "inet/power/base/PowerDefs.h"

namespace inet {

namespace power {

class IEnergySource;

/**
 * This class is a base interface that must be implemented by energy consumer
 * models to integrate with other parts of the power model. This interface is
 * extended by various energy consumer interfaces. Actual energy consumer
 * implementations should implement one of the derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 */
class INET_API IEnergyConsumer
{
  public:
    virtual ~IEnergyConsumer() {}

    /**
     * Returns the energy source that provides energy for this energy consumer.
     * This function never returns nullptr.
     */
    virtual IEnergySource *getEnergySource() const = 0;
};

} // namespace power

} // namespace inet

#endif

