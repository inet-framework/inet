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
        // Set only for an HT-or-later local STA when peer capability management establishes that the peer supports Compressed Block Ack.
        // This explicit assumption is needed because peer HT capabilities are not represented by the baseline agreement contract.
        bool assumePeerSupportsCompressedBlockAck = default(false);
        @display("i=block/control");
}
