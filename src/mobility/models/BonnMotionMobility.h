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

#ifndef BONN_MOTION_MOBILITY_H
#define BONN_MOTION_MOBILITY_H

#include "INETDefs.h"

#include "LineSegmentsMobilityBase.h"
#include "BonnMotionFileCache.h"


/**
 * @brief Uses the BonnMotion native file format. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API BonnMotionMobility : public LineSegmentsMobilityBase
{
  protected:
    // state
    bool is3D;
    const BonnMotionFile::Line *lines;
    int currentLine;

  protected:
    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage);

    /** @brief Initializes the position according to the mobility model. */
    virtual void initializePosition();

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void setTargetPosition();

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void move();

  public:
    BonnMotionMobility();

    virtual ~BonnMotionMobility();
};

#endif
