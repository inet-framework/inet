/*
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
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


#ifndef STATIC_GRID_MOBILITY_H
#define STATIC_GRID_MOBILITY_H

#include "INETDefs.h"

#include "StationaryMobility.h"


/**
 * @brief Mobility model which places all hosts at constant distances
 *  within the simulation area (resulting in a regular grid).
 *
 * @ingroup mobility
 * @author Isabel Dietrich
 */
class INET_API StaticGridMobility : public StationaryMobility
{
  protected:
    double marginX;
    double marginY;
    int numHosts;

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage);

    /** @brief Initializes the position according to the mobility model. */
    virtual void initializePosition();

    /** @brief Save the host position. */
    virtual void finish();

  public:
    StaticGridMobility();
};

#endif
