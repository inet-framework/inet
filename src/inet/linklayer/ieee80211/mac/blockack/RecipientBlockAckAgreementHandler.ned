//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

//
// Manages Block Acknowledgment (Block Ack) agreements for the recipient side
// in IEEE 802.11 networks. Processes ADDBA requests from originators, creates
// and maintains Block Ack agreements, handles timeouts, and generates appropriate
// responses. Implements the recipient side of the Block Ack mechanism as defined
// in the IEEE 802.11 standard for efficient acknowledgment of multiple frames.
//
simple RecipientBlockAckAgreementHandler
{
    parameters:
        string rateSelectionModule;

        @display("i=block/cogwheel");
}

