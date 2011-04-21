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


#ifndef RESTRICTED_CONST_SPEED_MOBILITY_H
#define RESTRICTED_CONST_SPEED_MOBILITY_H

#include <omnetpp.h>

#include "ConstSpeedMobility.h"


/**
 * @brief Controls all movement related things of a host
 *
 * Parameters to be specified in omnetpp.ini
 *  - vHost : Speed of a host [m/s]
 *  - updateInterval : Time interval to update the hosts position
 *  - x, y : Starting position of the host, -1 = random
 *
 * @ingroup mobility
 * @author Steffen Sroka, Marc Loebbers, Daniel Willkomm
 * @sa ChannelControl
 */
class INET_API RestrictedConstSpeedMobility : public ConstSpeedMobility
{
  protected:
	double x1;
	double y1;
	double x2;
	double y2;


  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);
    virtual Coord getRandomPosition();
};

#endif
