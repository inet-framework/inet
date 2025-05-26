//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.contract;

//
// Module interface for ground models that define the terrain in physical
// environment simulations. Provides methods to compute ground projections and
// normals for any position, which is necessary for accurate modeling of signal
// propagation and mobility over terrain. Implementations range from simple flat
// surfaces to terrain models based on real-world elevation data.
//
moduleinterface IGround
{
    parameters:
        @display("i=block/table");
}

