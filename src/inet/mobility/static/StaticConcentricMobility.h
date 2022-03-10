//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#ifndef __INET_STATICCONCENTRICMOBILITY_H
#define __INET_STATICCONCENTRICMOBILITY_H

#include "inet/mobility/base/StationaryMobilityBase.h"

namespace inet {

/**
 * @brief Mobility model which places all hosts on concenctric circles
 *
 * @ingroup mobility
 * @author Florian Meier
 */
class INET_API StaticConcentricMobility : public StationaryMobilityBase
{
  protected:
    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

  public:
    StaticConcentricMobility() {};
};

} // namespace inet

#endif

