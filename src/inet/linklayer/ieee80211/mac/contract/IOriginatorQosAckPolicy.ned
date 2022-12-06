//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

moduleinterface IOriginatorQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

