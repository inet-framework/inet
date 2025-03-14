//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IOriginatorAckPolicy;


//
// Implements the standard IEEE 802.11 originator Acknowledgment (ACK) policy for 
// non-QoS stations. This module determines whether an ACK frame should be expected 
// after transmitting a data frame and calculates the appropriate timeout period 
// for waiting for the ACK. The policy follows the basic 802.11 acknowledgment rules, 
// where unicast data frames require acknowledgment while broadcast/multicast frames 
// do not. It also handles control frames that don't require acknowledgment and 
// manages the ACK timeout calculation based on the current rate selection.
//
// @see ~IOriginatorAckPolicy, ~OriginatorQosAckPolicy, ~RecipientAckPolicy
//
simple OriginatorAckPolicy like IOriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        double ackTimeout @unit(s) = default(-1s);
        @display("i=block/control");
}
