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
        double PL0;
        double muGamma;
        double muSigma;
        double sigmaGamma;
        double sigmaSigma;
        bool shadowing;
        @class(UwbIrStochasticPathLoss);
        @display("i=block/control");
}

