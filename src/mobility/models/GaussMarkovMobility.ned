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

package inet.mobility.models;

import inet.mobility.MobilityBase;


simple GaussMarkovMobility extends MobilityBase
{
    parameters:
        double updateInterval @unit("s") = default(0.1s);
        double alpha = default(0.5); // [0;1], 0 - brown motion, 1 - linear motion
        double speed @unit("mps") = default(10mps); // meter/sec
        double angle @unit("deg") = default(0deg);
        double variance = default(40);
        int margin @unit("m") = default(30m);
        @class(GaussMarkovMobility);
}
