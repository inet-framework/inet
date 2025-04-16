//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Module interface for IEEE 802.11 agent modules that provide the decision
// logic for selecting access points and managing wireless connections in 
// wireless stations. Agents initiate channel scanning, association,
// authentication, and handovers by sending commands to the management module.
//
moduleinterface IIeee80211Agent
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input mgmtIn;
        output mgmtOut;
}

