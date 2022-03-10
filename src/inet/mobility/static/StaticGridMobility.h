//
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#ifndef __INET_STATICGRIDMOBILITY_H
#define __INET_STATICGRIDMOBILITY_H

#include "inet/mobility/base/StationaryMobilityBase.h"

namespace inet {

/**
 * @brief Mobility model which places all hosts at constant distances
 *  within the simulation area (resulting in a regular grid).
 *
 * @ingroup mobility
 * @author Isabel Dietrich
 */
class INET_API StaticGridMobility : public StationaryMobilityBase
{
  protected:
    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

  public:
    StaticGridMobility() {};
};

} // namespace inet

#endif

