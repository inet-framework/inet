//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

simple RecipientQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

