//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Module interface for all IEEE 802.11 management module types.
// It exists to specify what gates a management module should have
// in order to be usable within ~Ieee80211Interface.
//
moduleinterface IIeee80211Mgmt
{
    parameters:
        @display("i=block/cogwheel");
        string macModule;               // The path to the MAC module
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn;   // optional gates used for the agent in stations
        output agentOut; // optional gates used for the agent in stations
}

