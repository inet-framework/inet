//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.propagation;

import inet.physicallayer.wireless.common.base.packetlevel.PropagationBase;

//
// This propagation model computes the propagation time to be independent of
// the traveled distance. In other words, the propagation time is determined by
// a constant parameter.
//
module ConstantTimePropagation extends PropagationBase
{
    parameters:
        double propagationTime @unit(s) = default(0s); // the time it takes for a signal to propagate from any transmitter to any receiver
        @class(ConstantTimePropagation);
}

