//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11e collision controllers used with EDCA. Detects and 
// manages internal collisions that occur when multiple access categories (ACs) 
// complete their backoff procedures simultaneously. Implements the collision 
// resolution mechanism that grants channel access to the highest priority AC 
// and triggers a backoff procedure for lower priority ACs, as if an external 
// collision had occurred.
//
// @see ~EdcaCollisionController, ~Edca, ~Edcaf
//
moduleinterface ICollisionController
{
    parameters:
        @display("i=block/switch");
}
