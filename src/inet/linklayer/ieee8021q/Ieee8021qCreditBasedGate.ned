//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
