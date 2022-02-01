//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.policy;

import inet.protocolelement.fragmentation.contract.IFragmenterPolicy;

simple LengthBasedFragmenterPolicy like IFragmenterPolicy
{
    parameters:
        int minFragmentLength @unit(b) = default(0b);
        int maxFragmentLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        int fragmentHeaderLength @unit(b) = default(0b);
        @class(LengthBasedFragmenterPolicy);
}
