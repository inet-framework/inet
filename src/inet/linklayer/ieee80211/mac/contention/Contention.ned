//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ieee80211.mac.contention;

import inet.linklayer.ieee80211.mac.contract.IContention;

//
// The default implementation of IContention.
//
simple Contention like IContention
{
    parameters:
        bool initialChannelBusy = default(true); // assume that channel was busy before the simulation started
        bool backoffOptimization = default(true);    // we can pretend the frame has arrived into the queue a little bit earlier, and may be able to start transmitting immediately
        @display("i=block/timer");
        @signal[stateChanged](type=long); // TODO: enum=inet::ieee80211::Contention::State
        @statistic[state](title="contention state"; source=stateChanged; record=vector; interpolationmode=none);
}
