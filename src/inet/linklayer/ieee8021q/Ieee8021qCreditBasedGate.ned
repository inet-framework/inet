//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.gate.CreditBasedGate;

//
// This module is a packet gate that can be used to implement the IEEE 802.1q
// credit based shaper algorithm in combination with a packet queue.
//
simple Ieee8021qCreditBasedGate extends CreditBasedGate
{
    parameters:
        double idleSlope @unit(bps); // data rate of the accumulated credits when the transmission channel is idle
        double sendSlope @unit(bps) = default(idleSlope - bitrate); // data rate of the spent credits when the transmission channel is in use
        double maxInterferenceSize @unit(b) = default(inf b);
        idleCreditGainRate = default(dropUnit(idleSlope));
        transmitCreditSpendRate = default(-dropUnit(sendSlope));
        maxCredit = default(dropUnit(maxInterferenceSize) * dropUnit(idleSlope / bitrate));
}
