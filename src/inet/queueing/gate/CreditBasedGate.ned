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

simple CreditBasedGate extends PacketGateBase like IPacketGate
{
    parameters:
        double idleCreditGainRate;
        double transmitCreditSpendRate;
        double initialCredit = default(0);
        double transmitCreditLimit = default(0);
        double minCredit = default(-inf);
        double maxCredit = default(inf);
        displayStringTextFormat = default("contains %n cr\nserved %p pk (%l)"); // determines display string text above the submodule
        @class(CreditBasedGate);
        @signal[currentCreditChanged](type=double);
        @statistic[currentCredit](title="current credit"; source=currentCreditChanged; record=vector; interpolationmode=linear);
}
