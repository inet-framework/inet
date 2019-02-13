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

package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IRx;

//
// The default implementation of ~IRx.
//
simple Rx like IRx
{
    parameters:
        @display("i=block/rx");
        @signal[navChanged](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[nav](title="network allocation vector"; source=navChanged; record=vector; interpolationmode=linear);
}

