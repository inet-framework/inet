//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation.policy;

import inet.common.SimpleModule;
import inet.protocolelement.fragmentation.contract.IFragmenterPolicy;

//
// Implements a fragmentation policy based on packet length constraints.
// Determines fragment sizes based on configurable minimum and maximum fragment
// lengths, with options for length rounding and accounting for fragment header
// overhead.
//
simple LengthBasedFragmenterPolicy extends SimpleModule like IFragmenterPolicy
{
    parameters:
        @class(LengthBasedFragmenterPolicy);
        int minFragmentLength @unit(b) = default(0b);
        int maxFragmentLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        int fragmentHeaderLength @unit(b) = default(0b);
        @class(LengthBasedFragmenterPolicy);
}
