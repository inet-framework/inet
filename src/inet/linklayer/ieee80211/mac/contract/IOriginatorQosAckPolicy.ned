//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for originator QoS ACK policies.
//
moduleinterface IOriginatorQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

