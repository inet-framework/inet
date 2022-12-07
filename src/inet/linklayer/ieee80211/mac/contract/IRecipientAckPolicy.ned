//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for recipient ACK policies. This policy decides whether
// a received frame needs to be answered with an ACK frame.
//
moduleinterface IRecipientAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

