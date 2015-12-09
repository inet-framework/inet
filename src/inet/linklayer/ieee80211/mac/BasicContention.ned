//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Author: Andras Varga
//

package inet.linklayer.ieee80211.mac;

//
// The default implementation of IContention.
//
simple BasicContention like IContention
{
    parameters:
        string macModule;
        string upperMacModule;
        string collisionControllerModule;
        string rxModule;
        string statisticsModule;
        bool initialBackoffOptimization = default(true);    // we can pretend the frame has arrived into the queue a little bit earlier, and may be able to start transmitting immediately
        @display("i=block/control");
        @signal[stateChanged](type=long); // TODO: enum=inet::ieee80211::BasicContention::State
        @statistic[state](title="contention state"; source=stateChanged; record=vector; interpolationmode=none);
}
