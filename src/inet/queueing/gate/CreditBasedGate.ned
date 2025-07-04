//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// A packet gate that operates based on the number of credits
// it contains. The gate is open if the number of credits is greater than the
// configured transmit credit limit, otherwise it is closed. The module attaches
// a ~CreditGateTag to all packets that pass through. The attached tag allows
// the gate module to determine if a packet belongs to it when the packet is
// eventually transmitted.
//
// The number of credits decreases if a packet that passed through this gate is
// currently being transmitted by the network interface. Otherwise, the number
// of credits increases below the transmit credit limit or if there are packets
// waiting to be transmitted. Besides, the number of credits immediately drops
// down to the transmit credit limit if no more packets are available after the
// network interface finishes a transmission from this gate.
//
// This module is part of the credit-based shaper infrastructure.
//
// @see ~CreditGateTag, ~PacketQueue
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
        displayStringTextFormat = default("contains {currentCredit} cr, rate {currentCreditGainRate} crps\nserved %p pk (%l)"); // Determines display string text above the submodule
        @class(CreditBasedGate);
        @signal[creditsChanged](type=double);
        @statistic[numCredits](title="number of credits"; source=creditsChanged; record=vector; unit=cr; interpolationmode=linear);
}
