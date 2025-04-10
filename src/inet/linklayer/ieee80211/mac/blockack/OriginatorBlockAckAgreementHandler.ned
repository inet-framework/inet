//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.common.SimpleModule;

simple OriginatorBlockAckAgreementHandler extends SimpleModule
{
    parameters:
        @class(OriginatorBlockAckAgreementHandler);
        string rateSelectionModule;

        @display("i=block/cogwheel");
}
