//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Path loss model that accounts for interference between direct and
// ground-reflected signal paths. Considers phase differences, polarization
// effects (horizontal or vertical), and ground permittivity to accurately model
// constructive and destructive interference patterns, yielding a more accurate
// approximation than ~TwoRayGroundReflection even on short distances.
//
// Based on research by Sommer and Dressler, this model is particularly suitable
// for vehicular network simulations where ground reflections significantly
// impact signal propagation.
//
// @see TwoRayGroundReflection
//
module TwoRayInterference extends Module like IPathLoss
{
    parameters:
        @class(TwoRayInterference);
        @display("i=block/control");
        double epsilon_r = default(1.02);
        string polarization = default("horizontal");
}

