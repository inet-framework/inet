//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.IRecipientAckPolicy;


//
// Implements the standard IEEE 802.11 recipient Acknowledgment (ACK) policy for 
// non-QoS stations. This module determines whether to send an ACK frame in response 
// to a received data frame based on the frame's characteristics. Following the 
// basic 802.11 acknowledgment rules, it generates ACK responses for correctly 
// received unicast data frames addressed to this station, while not acknowledging 
// broadcast/multicast frames or frames with errors. The policy also handles control 
// frames that don't require acknowledgment and ensures proper rate selection for 
// ACK frames.
//
// @see ~IRecipientAckPolicy, ~RecipientQosAckPolicy, ~OriginatorAckPolicy
//
simple RecipientAckPolicy like IRecipientAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
