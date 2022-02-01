//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

simple OriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        double ackTimeout @unit(s) = default(-1s);
        @display("i=block/control");
}

