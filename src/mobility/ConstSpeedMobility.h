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


#ifndef CONST_SPEED_MOBILITY_H
#define CONST_SPEED_MOBILITY_H

#include <omnetpp.h>

#include "BasicMobility.h"


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
class INET_API ConstSpeedMobility : public BasicMobility
{
  protected:
    /** @brief Velocity of the host*/
    double vHost;

    /** @brief Time interval to update the hosts position*/
    double updateInterval;

    /** @brief If true, the host doesn't move*/
    bool stationary;

    /** @brief parameters to handle the movement of the host*/
    /*@{*/
    Coord targetPos;
    Coord stepSize;
    int numSteps;
    int step;
    /*@}*/

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Calculate the target position to move to*/
    virtual void setTargetPosition();

    /** @brief Move the host*/
    virtual void move();
};

#endif
