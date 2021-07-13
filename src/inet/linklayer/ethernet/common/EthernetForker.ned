//
// Copyright (C) 2012 OpenSim Ltd.
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

package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// send local delivery frames to out[0], forwardable frames to out[1]
//
simple EthernetForker extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(EthernetForker);
        @display("i=block/classifier");
        string bridgeAddress = default(""); // MacAddress
        string interfaceTableModule;
        object localDeliveryMacAddresses = default([["01:80:C2:00:00:00", "01:80:C2:00:00:0F"]]);   // array of arrays of MAC addresses, one inner array contains 1 or 2 MAC addresses: [[a,b], [c]] means a..b, c
}

