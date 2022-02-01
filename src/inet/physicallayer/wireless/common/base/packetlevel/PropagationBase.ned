//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IPropagation;

//
// This module servces as a base module for propagation models.
//
module PropagationBase like IPropagation
{
    parameters:
        double propagationSpeed @unit(mps) = default(299792458mps); // the theoretical propagation speed of signals on the medium (used by path loss models)
        @display("i=block/timer");
}

