//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.scalar;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IMediumAnalogModel;

//
// This analog model computes with scalar analog power representation. In other
// words, the signal power does not change over time or frequency, except for
// the boundaries of the signal.
//
module ScalarMediumAnalogModel extends Module like IMediumAnalogModel
{
    parameters:
        bool ignorePartialInterference = default(false); // Enabled means interference that only partially overlaps with the signal being received is ignored; otherwise, the simulation stops with an error
        @display("i=block/tunnel");
        @class(ScalarMediumAnalogModel);
}

