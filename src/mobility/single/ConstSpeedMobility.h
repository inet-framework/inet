/* -*- mode:c++ -*- ********************************************************
 * file:        ConstSpeedMobility.h
 *
 * author:      Steffen Sroka
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/

#ifndef __INET_CONSTSPEEDMOBILITY_H
#define __INET_CONSTSPEEDMOBILITY_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/common/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Moves along a line with constant speed to a randomly chosen target.
 * When the target is reached it selects randomly a new one.
 *
 * @ingroup mobility
 * @author Steffen Sroka, Marc Loebbers, Daniel Willkomm
 */
class INET_API ConstSpeedMobility : public LineSegmentsMobilityBase
{
  protected:
    /** @brief Speed parameter. */
    double speed;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage);

    /** @brief Calculate a new target position to move to. */
    virtual void setTargetPosition();

  public:
    virtual double getMaxSpeed() const { return speed; }
    ConstSpeedMobility();
};

} // namespace inet

#endif // ifndef __INET_CONSTSPEEDMOBILITY_H

