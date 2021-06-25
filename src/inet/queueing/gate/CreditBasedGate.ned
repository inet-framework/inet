//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.queueing.gate;

import inet.queueing.base.PacketGateBase;
import inet.queueing.contract.IPacketGate;

//
// This module is a packet gate that operates based on the number of credits
// it contains. The gate is open if the number of credits is greater than the
// configured credit limit, it is closed otherwise. The module attaches a
// ~CreditGateTag to all packets that pass through. The attached tag allows
// the gate module to determine if a packet belongs to it when the packet is
// eventually transmitted. The number of credits decreases over time when a
// packet that passed through this gate is being transmitted, it increases
// otherwise.
//
// This module is part of the credit based shaper infrastructure.
//
// @see ~CreditGateTag, ~PacketQueue
//
simple CreditBasedGate extends PacketGateBase like IPacketGate
{
    parameters:
        double idleCreditGainRate; // rate of credit decrease while the packets that passed through this gate don't use the physical channel
        double transmitCreditSpendRate; // rate of credit increase while the packets that passed through this gate use the physical channel
        double initialCredit = default(0); // initial number of credits
        double transmitCreditLimit = default(0); // credit limit above which the gate is open 
        double minCredit = default(-inf); // minimum number of credits
        double maxCredit = default(inf); // maximum number of credits
        displayStringTextFormat = default("contains %n cr\nserved %p pk (%l)"); // determines display string text above the submodule
        @class(CreditBasedGate);
        @signal[currentCreditChanged](type=double);
        @statistic[currentCredit](title="current credit"; source=currentCreditChanged; record=vector; interpolationmode=linear);
}
