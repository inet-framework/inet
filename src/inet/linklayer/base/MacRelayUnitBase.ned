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

package inet.linklayer.base;

import inet.linklayer.contract.IMacRelayUnit;

simple MacRelayUnitBase
{
    parameters:
        string macTableModule = default("^.macTable");
        string interfaceTableModule = default("^.interfaceTable");
        string displayStringTextFormat = default("proc: %p\ndisc: %d");
        @display("i=block/switch");
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromUpper](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input ifIn;
        output ifOut;
        input upperLayerIn;
        output upperLayerOut;
}

