/* -*- mode:c++ -*- ********************************************************/
//
// Copyright (C) 2007 Peterpaul Klein Haneveld
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef RECTANGLE_MOBILITY_H
#define RECTANGLE_MOBILITY_H


#include <LineSegmentsMobilityBase.h>


/**
 * @brief Tractor movement model. See NED file for more info.
 *
 * NOTE: Does not yet support 3-dimensional movement.
 * @ingroup mobility
 * @author Peterpaul Klein Haneveld
 */
class TractorMobility : public LineSegmentsMobilityBase
{
  protected:
    double speed; //< speed along the trajectory
    double x1, y1, x2, y2; ///< rectangle bounds of the field
    int rowCount; ///< the number of rows that the tractor must take
    int step;

  protected:
    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int);

    /** @brief Initializes the position according to the mobility model. */
    virtual void initializePosition();

    /** @brief Calculate a new target position to move to. */
    void setTargetPosition();

  public:
    TractorMobility();
};

#endif
