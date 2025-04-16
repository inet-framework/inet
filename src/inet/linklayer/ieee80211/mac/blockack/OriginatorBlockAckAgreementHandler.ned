//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.common.SimpleModule;

//
// Manages Block Acknowledgment (Block Ack) agreements for the originator side
// in IEEE 802.11 networks. Handles the creation, maintenance, and termination
// of Block Ack agreements with receivers. Processes ADDBA requests/responses
// and DELBA frames, tracks agreement timeouts, and maintains the state of
// active agreements. Enables efficient acknowledgment of multiple frames with
// a single Block Ack frame.
//
simple OriginatorBlockAckAgreementHandler extends SimpleModule
{
    parameters:
        @class(OriginatorBlockAckAgreementHandler);
        string rateSelectionModule;

        @display("i=block/cogwheel");
}
