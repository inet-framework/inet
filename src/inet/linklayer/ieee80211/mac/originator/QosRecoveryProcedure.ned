//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

simple QosRecoveryProcedure
{
    parameters:
        string cwCalculatorModule;
        string rtsPolicyModule;

        int shortRetryLimit = default(7);
        int longRetryLimit = default(4);
        @display("i=block/cogwheel");
        @signal[contentionWindowChanged](type=long);
        @signal[retryLimitReached](type=inet::Packet);
        @statistic[contentionWindowChanged](title="contention window"; record=vector; interpolationmode=sample-hold);
        @statistic[retryLimitReached](title="retry limit reached"; record=count);
}

