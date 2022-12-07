//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.IRecipientQosAckPolicy;


//
// Implements the default recipient ACK policy for QoS stations.
//
simple RecipientQosAckPolicy like IRecipientQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}

