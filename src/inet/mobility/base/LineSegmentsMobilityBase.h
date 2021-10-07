//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LINESEGMENTSMOBILITYBASE_H
#define __INET_LINESEGMENTSMOBILITYBASE_H

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
 */
class INET_API LineSegmentsMobilityBase : public MovingMobilityBase
{
  protected:
    /** @brief End position of current linear movement. */
    Coord targetPosition;

    /** @brief The velocity in current segment. */
    Coord segmentVelocity;

    /** @brief Start position of current linear movement. */
    Coord segmentStartPosition;

    /** @brief Start time of current linear movement. */
    simtime_t segmentStartTime;

    BorderPolicy borderPolicy = RAISEERROR;

  protected:
    virtual void initializePosition() override;

    // helper function: call setTargetPosition() and save segment start infos.
    void doSetTargetPosition();

    virtual void move() override;

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetPosition and nextChange has been reached, and its task is
     * to set a new targetPosition and nextChange. At the end of the movement
     * sequence, it should set nextChange to -1.
     */
    virtual void setTargetPosition() = 0;

    /** @brief processing border policy. */
    virtual void processBorderPolicy();

  public:
    LineSegmentsMobilityBase();
};

} // namespace inet

#endif

