//***************************************************************************
// * file:        ConstSpeedMobility.cc
// *
// * author:      Steffen Sroka
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Uses a probabilistic transition matrix to change the state of motion.
// In this model, the state of the mobile node in each direction (x and y) can be:
//  - the node stays in its current position
//  - the node moves forward
//  - the node moves backward
//
simple ChiangMobility extends MovingMobilityBase
{
    parameters:
        double stateTransitionUpdateInterval @unit(s);
        double speed @unit(mps);
        @class(ChiangMobility);
}
