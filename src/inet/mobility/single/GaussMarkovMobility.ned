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
// Uses a Gauss-Markov model to control the randomness in the movement.
// Totally random walk (Brownian motion) is obtained by setting alpha=0,
// while alpha=1 results a linear motion.
//
simple GaussMarkovMobility extends MovingMobilityBase
{
    parameters:
        @class(GaussMarkovMobility);
        double alpha = default(0.5); // [0;1], 0 - brown motion, 1 - linear motion
        double speed @unit(mps);
        double speedStdDev @unit(mps);     // speed standard deviation
        double angle @unit(deg) = default(uniform(0deg, 360deg));
        double angleStdDev @unit(deg);     // angle standard deviation
        double margin @unit(m);
}

