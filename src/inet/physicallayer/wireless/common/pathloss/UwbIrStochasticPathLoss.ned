//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module UwbIrStochasticPathLoss like IPathLoss
{
    parameters:
        double PL0; // Reference path loss at reference distance
        double muGamma; // Mean path loss exponent
        double muSigma; // Mean standard deviation of shadowing [dB]
        double sigmaGamma; // Standard deviation of path loss exponent
        double sigmaSigma; // Standard deviation of shadowing
        bool shadowing; // Enable/disable shadowing
        @class(UwbIrStochasticPathLoss);
        @display("i=block/control");
}

