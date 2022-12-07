//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for RTS policies.
//
moduleinterface IRtsPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

