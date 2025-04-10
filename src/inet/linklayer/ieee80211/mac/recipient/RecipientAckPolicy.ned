//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRecipientAckPolicy;

//
// Implements the default recipient ACK policy.
//
simple RecipientAckPolicy extends SimpleModule like IRecipientAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

