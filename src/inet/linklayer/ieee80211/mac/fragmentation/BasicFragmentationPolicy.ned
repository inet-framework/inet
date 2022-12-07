//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.fragmentation;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;

//
// Implements a basic fragmentation policy, which employs a fragmentation
// frame size threshold.
//
simple BasicFragmentationPolicy like IFragmentationPolicy
{
    parameters:
        int fragmentationThreshold @unit(B) = default(1500B);
        @display("i=block/control");
}

