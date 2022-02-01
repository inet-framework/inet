//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.antenna;

import inet.physicallayer.wireless.common.base.packetlevel.AntennaBase;

//
// This antenna model computes the antenna gain from the direction of the signal
// using linear interpolation for all 3 euler angles independently of each other.
// The gain parameters contain a sequence of angle [degree] and gain [dB] pairs.
// The first pair must be at 0 [degree].
//
//# TODO refactor to use Delaunay triangulation on unit sphere, i.e. triangulate
//# result from enclosing spherical triangle as seen from the center
//
module InterpolatingAntenna extends AntennaBase
{
    parameters:
        string headingGains = default("0 0");   // sequence of angle [degree] and gain [dB] pairs
        string elevationGains = default("0 0"); // sequence of angle [degree] and gain [dB] pairs
        string bankGains = default("0 0");      // sequence of angle [degree] and gain [dB] pairs
        @class(InterpolatingAntenna);
}

