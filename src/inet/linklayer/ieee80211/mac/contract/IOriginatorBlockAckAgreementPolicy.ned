//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

moduleinterface IOriginatorBlockAckAgreementPolicy
{
    parameters:
        string originatorAckPolicyModule;
        @display("i=block/control");
}

