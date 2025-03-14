//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// Implements a credit-based shaper algorithm. Controls packet flow based on a
// credit system that ensures bandwidth allocation while limiting traffic bursts.
// This module requires to be placed inside a NetworkInterface module.
//
// Operation mechanism:
// - The gate is open when credits exceed the transmit credit limit
// - Credits increase at idleCreditGainRate when the flow is idle
// - Credits decrease at transmitCreditSpendRate when the flow is transmitting
// - Credits are bounded by minCredit and maxCredit parameters
// - When transmission completes with no waiting packets, credits reset to transmitCreditLimit
//
// The module attaches a ~CreditGateTag to all packets that pass through, allowing
// it to track which packets belong to its flow when they reach the network interface
// for transmission. This tracking is essential for proper credit accounting.
//
// Key features:
// - Provides guaranteed bandwidth allocation for time-sensitive streams
// - Prevents traffic bursts through credit-based flow control
// - Supports configurable credit rates and limits
//
// @see ~Ieee8021qCreditBasedGate, ~CreditGateTag, ~PacketQueue, ~CreditBasedShaper
//
simple CreditBasedGate extends PacketGateBase like IPacketGate
{
    parameters:
        double idleCreditGainRate; // Rate of credit increase while the packets that passed through this gate don't use the physical channel
        double transmitCreditSpendRate; // Rate of credit decrease while the packets that passed through this gate use the physical channel
        double initialCredit = default(0); // Initial number of credits
        double transmitCreditLimit = default(0); // Credit limit above which the gate is open
        double minCredit = default(-inf); // Minimum number of credits
        double maxCredit = default(inf); // Maximum number of credits
        bool accumulateCreditInGuardBand = default(false); // Whether credit increases during implicit guard band when no packets can be sent anymore
        displayStringTextFormat = default("contains %n cr\nserved %p pk (%l)"); // Determines display string text above the submodule
        @class(CreditBasedGate);
        @signal[creditsChanged](type=double);
        @statistic[numCredits](title="number of credits"; source=creditsChanged; record=vector; unit=cr; interpolationmode=linear);
}
