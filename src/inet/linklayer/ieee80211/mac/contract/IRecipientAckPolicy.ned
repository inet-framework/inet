//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 recipient Acknowledgment (ACK) policies that determine 
// how to respond to received data frames. These policies decide whether to send 
// an ACK frame in response to a received data frame based on frame characteristics 
// and network conditions. Recipient ACK policies are responsible for implementing 
// the frame acknowledgment mechanism from the receiver's perspective, which is 
// essential for reliable data transmission in wireless networks. The policy ensures 
// that unicast frames are properly acknowledged while broadcast/multicast frames 
// are not.
//
// @see ~RecipientAckPolicy, ~IRecipientQosAckPolicy, ~IOriginatorAckPolicy
//
moduleinterface IRecipientAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
