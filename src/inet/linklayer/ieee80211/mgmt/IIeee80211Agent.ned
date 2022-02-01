//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

moduleinterface IIeee80211Agent
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input mgmtIn;
        output mgmtOut;
}

