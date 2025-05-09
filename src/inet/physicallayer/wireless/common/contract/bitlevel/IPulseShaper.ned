//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.bitlevel;

//
// Interface for pulse shaping components in wireless communications. Transforms
// symbol-domain signals into sample-domain signals by applying pulse shaping to
// digital symbols, creating a continuous-time waveform. Used in detailed
// physical layer simulations.
//
// @see IPulseFilter
//
moduleinterface IPulseShaper
{
    parameters:
        @display("i=block/opticaldelay");
}

