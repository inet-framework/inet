//
// Copyright (C) OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ethernet.switch;

import inet.linklayer.contract.IMacRelayUnit;

simple MacRelayUnit like IMacRelayUnit
{
    parameters:
        @display("i=block/switch");
        string macTableModule = default("^.macTable"); // The path to the MacAddressTable module
        string interfaceTableModule = default("^.interfaceTable"); // The path to the InterfaceTable module
        bool hasStp;
        string displayStringTextFormat = default("proc: %p\ndisc: %d");
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
    gates:
        input ifIn @labels(EtherFrame);
        output ifOut @labels(EtherFrame);
        input upperLayerIn;
        output upperLayerOut;
}

