//
// Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_STATICLINEARMOBILITY_H
#define __INET_STATICLINEARMOBILITY_H

#include "inet/mobility/base/StationaryMobilityBase.h"

namespace inet {

/**
 * @brief Mobility model which places all hosts at constant distances
 * in a line with an orientation
 *
 * @ingroup mobility
 * @author Alfonso Ariza
 */
class INET_API StaticLinearMobility : public StationaryMobilityBase
{
  protected:

    double separation;
    double initialX;
    double initialY;
    rad orientation;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Save the host position. */
    virtual void finish() override;

  public:
    StaticLinearMobility();
};

} // namespace inet

#endif

