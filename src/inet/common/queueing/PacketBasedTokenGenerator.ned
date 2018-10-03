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

package inet.common.queueing;

import inet.common.queueing.base.PacketSinkBase;
import inet.common.queueing.contract.IPacketConsumer;

simple PacketBasedTokenGenerator extends PacketSinkBase like IPacketConsumer
{
    parameters:
        string serverModule;
        int numTokensPerPacket = default(1);
        int numTokensPerBit = default(0);
        displayStringTextFormat = default("processed %p pk (%l)\ngenerated %t tk"); // %p number of packet, %l total length
        @class(PacketBasedTokenGenerator);
        @display("i=block/control");
    gates:
        input in;
}
