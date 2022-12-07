//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for originator ACK policies. This policy tells whether
// an ACK is expected for a transmitted frame, and also determines
// the ACK timeout.
//
moduleinterface IOriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

