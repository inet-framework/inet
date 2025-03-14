//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 Hybrid Coordination Function (HCF) implementations. 
// HCF combines contention-based and controlled channel access methods, providing 
// both EDCA (Enhanced Distributed Channel Access) for prioritized traffic and 
// HCCA (HCF Controlled Channel Access) for parameterized QoS. Supports traffic 
// differentiation through multiple access categories and quality of service 
// enhancements introduced in IEEE 802.11e.
//
// @see ~Hcf, ~Edca, ~Edcaf, ~Hcca
//
moduleinterface IHcf
{
    parameters:
        @display("i=block/layer");
}
