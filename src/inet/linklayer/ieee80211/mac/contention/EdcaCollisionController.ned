//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contention;

import inet.linklayer.ieee80211.mac.contract.ICollisionController;

//
// IEEE 802.11e EDCA collision controller implementation that manages internal 
// collisions between different access categories. When multiple ACs complete 
// their backoff procedures simultaneously, it grants transmission opportunity 
// to the highest priority AC and schedules new backoff procedures for lower 
// priority ACs. This mechanism ensures that higher priority traffic classes 
// (like voice and video) receive preferential treatment over lower priority 
// traffic (like best effort and background).
//
// @see ~ICollisionController, ~Edca, ~Edcaf
//
simple EdcaCollisionController like ICollisionController
{
    parameters:
        @display("i=block/switch");
}
