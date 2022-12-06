//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

moduleinterface IOriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

