//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//***************************************************************************
// * file:        ConstSpeedMobility.cc
// *
// * author:      Steffen Sroka
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *
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
