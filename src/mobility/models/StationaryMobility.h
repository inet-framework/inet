//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef STATIONARY_MOBILITY_H
#define STATIONARY_MOBILITY_H

#include "INETDefs.h"

#include "MobilityBase.h"


/**
 * This mobility module does not move at all; it can be used for standalone stationary nodes.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API StationaryMobility : public MobilityBase
{
  protected:
    /** @brief Never happens. */
    virtual void handleSelfMessage(cMessage *message);

  public:
    /** @brief Returns the current position at the current simulation time. */
    virtual Coord getCurrentPosition() { return lastPosition; }

    /** @brief Returns the current speed at the current simulation time. */
    virtual Coord getCurrentSpeed() { return Coord::ZERO; }
};

#endif
