//
// Copyright (C) OpenSim Ltd.
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

package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// This module repeatedly pulls packets from the connected packet provider and
// it pushes them into the connected packet consumer. The packets are processed
// in zero simulation time, and the process continues until the available number
// of tokens is not sufficient anymore.
//
simple TokenBasedServer extends PacketServerBase like IPacketServer
{
    parameters:
        double initialNumTokens = default(0); // initial number of tokens
        double maxNumTokens = default(nan); // maximum number of tokens
        volatile double tokenConsumptionPerPacket = default(1); // how many tokens are consumed per processed packet
        volatile double tokenConsumptionPerBit = default(0); // how may tokens are consumed per processed bit
        displayStringTextFormat = default("contains %n tk\nserved %p pk (%l)"); // determines display string text above the submodule
        @signal[tokensAdded](type=double);
        @signal[tokensRemoved](type=double);
        @signal[tokensDepleted](type=double);
        @statistic[tokensAdded](title="tokens added"; record=sum; interpolationmode=none);
        @statistic[tokensRemoved](title="tokens removed"; record=sum; interpolationmode=none);
        @statistic[tokensDepleted](title="tokens depleted"; record=count; interpolationmode=none);
        @class(TokenBasedServer);
}
