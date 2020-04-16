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

package inet.queueing.tokengenerator;

import inet.queueing.base.PacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module generates tokens into the configured ~TokenBasedServer for each
// consumed packet.
//
simple PacketBasedTokenGenerator extends PacketSinkBase like IPassivePacketSink
{
    parameters:
        string serverModule; // module path of the packet server where the tokens are generated via C++ method call
        volatile double numTokensPerPacket = default(1); // number of tokens per consumed packet
        volatile double numTokensPerBit = default(0); // number of tokens per consumed bit
        displayStringTextFormat = default("processed %p pk (%l)\ngenerated %t tk\nto %s"); // %p number of packet, %l total length
        @class(PacketBasedTokenGenerator);
        @display("i=block/control");
        @signal[tokensCreated](type=double);
        @statistic[tokensCreated](title="tokens created"; record=count,sum,vector; interpolationmode=none);
    gates:
        input in;
}
