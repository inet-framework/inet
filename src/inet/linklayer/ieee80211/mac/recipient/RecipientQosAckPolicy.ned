//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IRecipientQosAckPolicy;

//
// Implements the default recipient ACK policy for QoS stations.
//
simple RecipientQosAckPolicy extends SimpleModule like IRecipientQosAckPolicy
{
    parameters:
        @class(RecipientQosAckPolicy);
        string rateSelectionModule;
        @display("i=block/control");
}

