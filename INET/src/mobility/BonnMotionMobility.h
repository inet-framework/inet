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

#ifndef BONNMOTION_MOBILITY_H
#define BONNMOTION_MOBILITY_H

#include <omnetpp.h>
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
    const BonnMotionFile::Line *vecp;
    int vecpos;

  protected:
    virtual ~BonnMotionMobility();

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void setTargetPosition();

    /** @brief Overridden from LineSegmentsMobilityBase.*/
    virtual void fixIfHostGetsOutside();
};

#endif

