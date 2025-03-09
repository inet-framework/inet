//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.policy;

import inet.protocolelement.fragmentation.contract.IFragmenterPolicy;

//
// Implements a fragmentation policy that divides packets into fragments based
// on configurable length constraints. Determines fragment sizes to ensure they
// meet network transmission requirements.
//
// Operation:
// - Calculates fragment sizes based on maximum fragment length parameter
// - Ensures fragments are not smaller than minimum fragment length
// - Rounds fragment lengths to specified boundaries for alignment
// - Accounts for fragment header overhead in size calculations
// - Provides consistent fragmentation decisions to fragmenter modules
//
// @see ~IFragmenterPolicy, ~FragmenterBase
//
simple LengthBasedFragmenterPolicy like IFragmenterPolicy
{
    parameters:
        int minFragmentLength @unit(b) = default(0b);
        int maxFragmentLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        int fragmentHeaderLength @unit(b) = default(0b);
        @class(LengthBasedFragmenterPolicy);
}
