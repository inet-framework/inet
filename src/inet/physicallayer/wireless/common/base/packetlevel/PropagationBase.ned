//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IPropagation;

//
// Serves as a base module for propagation models.
//
module PropagationBase extends Module like IPropagation
{
    parameters:
        double propagationSpeed @unit(mps) = default(299792458mps); // The theoretical propagation speed of signals on the medium (used by path loss models)
        @display("i=block/timer");
}
