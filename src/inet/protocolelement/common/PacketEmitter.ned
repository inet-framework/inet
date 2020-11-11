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

package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple PacketEmitter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string packetFilter = default("*"); // which packets are considered, matches all packets by default
        string packetDataFilter = default("*"); // which packets are considered based on the data they contain, matches all packets by default
        string direction @enum("undefined", "inbound", "outbound") = default("undefined");
        string signalName;
        @class(PacketEmitter);
        @display("i=block/star");
        @signal[packetSentToLower](type=Packet);
        @signal[packetReceivedFromLower](type=Packet);
}
