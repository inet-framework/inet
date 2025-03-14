//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.fragmentation;

import inet.linklayer.ieee80211.mac.contract.IFragmentationPolicy;

//
// Implements a threshold-based IEEE 802.11 fragmentation policy that divides frames 
// exceeding a configurable size limit into smaller fragments. When a frame's size 
// exceeds the fragmentationThreshold parameter, it is split into multiple fragments 
// that are transmitted and acknowledged separately. This approach improves transmission 
// reliability in error-prone environments by reducing the probability of frame corruption 
// and limiting retransmissions to only the affected fragments rather than the entire frame.
//
// @see ~IFragmentationPolicy
//
simple BasicFragmentationPolicy like IFragmentationPolicy
{
    parameters:
        int fragmentationThreshold @unit(B) = default(1500B);
        @display("i=block/control");
}
