//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 originator Acknowledgment (ACK) policies that determine 
// whether an ACK frame is expected for a transmitted data frame and calculate the 
// appropriate ACK timeout period. Originator ACK policies are responsible for 
// implementing the frame acknowledgment mechanism from the sender's perspective, 
// which is essential for reliable data transmission in wireless networks. These 
// policies help the MAC layer decide when to expect ACKs and how long to wait 
// before considering a transmission failed due to missing acknowledgment.
//
// @see ~OriginatorAckPolicy, ~IOriginatorQosAckPolicy, ~IRecipientAckPolicy
//
moduleinterface IOriginatorAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
