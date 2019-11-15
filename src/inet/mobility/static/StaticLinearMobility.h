/*
 *  Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

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

#endif // ifndef __INET_STATICLINEARMOBILITY_H

