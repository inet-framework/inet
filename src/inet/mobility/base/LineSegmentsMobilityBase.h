//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_LINESEGMENTSMOBILITYBASE_H
#define __INET_LINESEGMENTSMOBILITYBASE_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * @brief Base class for mobility models where movement consists of
 * a sequence of linear movements of constant speed.
 *
 * Subclasses must redefine setTargetPosition() which is supposed to set
 * a new targetPosition and nextChange once the previous target is reached.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API LineSegmentsMobilityBase : public MovingMobilityBase
{
  protected:
    /** @brief End position of current linear movement. */
    Coord targetPosition;

  protected:
    virtual void initializePosition() override;

    virtual void move() override;

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetPosition and nextChange has been reached, and its task is
     * to set a new targetPosition and nextChange. At the end of the movement
     * sequence, it should set nextChange to -1.
     */
    virtual void setTargetPosition() = 0;

  public:
    LineSegmentsMobilityBase();
};

} // namespace inet

#endif // ifndef __INET_LINESEGMENTSMOBILITYBASE_H

