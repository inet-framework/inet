//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Implements the Stanford University Interim (SUI) path loss model, an empirical
// model used to predict radio signal attenuation over various terrain types.
// Path loss is calculated based on distance, frequency, and antenna heights,
// with parameter sets defined for different environments.
//
// The model defines three terrain categories:
// - Terrain A: Highest path loss (dense urban or hilly areas)
// - Terrain B: Moderate path loss (suburban or mixed environments)
// - Terrain C: Lowest path loss (flat/rural areas with light vegetation)
//
module SuiPathLoss extends Module like IPathLoss
{
    parameters:
        @class(SuiPathLoss);
        @display("i=block/control");
        string terrain @enum("TerrainA", "TerrainB", "TerrainC"); // Type of terrain affecting path loss characteristics
        double transmitterAntennaHeight @unit(m);                  // Height of the transmitter antenna
        double receiverAntennaHeight @unit(m);                     // Height of the receiver antenna
}

