//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.channelaccess;

//
// Placeholder for the IEEE 802.11e HCF Controlled Channel Access (HCCA) mechanism. 
// HCCA provides parameterized QoS through centralized, scheduled channel access 
// controlled by a Hybrid Coordinator (HC). Unlike contention-based methods, HCCA 
// allows for guaranteed bandwidth, bounded latency, and jitter control for time-sensitive 
// applications.
//
// IMPORTANT: This module is currently a placeholder. INET does not yet implement 
// the HCCA functionality.
//
// @see ~Hcf, ~Edca
//
simple Hcca
{
    parameters:
        @display("i=block/star");
}
