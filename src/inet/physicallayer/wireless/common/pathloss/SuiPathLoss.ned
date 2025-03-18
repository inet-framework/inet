//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Implements the Stanford University Interim (SUI) path loss model, an empirical
// model for predicting radio signal attenuation in different environments.
// Calculates path loss based on distance, frequency, and antenna heights, with
// different parameter sets for various terrain types. Terrain types include:
// TerrainA (highest path loss, dense urban areas), TerrainB (intermediate path
// loss, suburban areas), and TerrainC (minimum path loss, flat/rural areas with
// light vegetation).
//
module SuiPathLoss like IPathLoss
{
    parameters:
        @class(SuiPathLoss);
        @display("i=block/control");
        string terrain @enum("TerrainA", "TerrainB", "TerrainC"); // Type of terrain affecting path loss characteristics
        double transmitterAntennaHeight @unit(m);                  // Height of the transmitter antenna
        double receiverAntennaHeight @unit(m);                     // Height of the receiver antenna
}
