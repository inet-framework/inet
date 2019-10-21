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

package inet.queueing.marker;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketSink;
import inet.queueing.contract.IPacketSource;

//
// This module attaches various request tags to packets.
//
simple PacketTagger extends PacketProcessorBase like IPacketSource, IPacketSink
{
    parameters:
        string filterClass = default("inet::queueing::AnyPacketFilter"); // determines which packets are tagged
        string interfaceName = default(""); // specifies outgoing interface, unused by default
        int hopLimit = default(-1); // specifies network layer TTL, unused by default
        int vlanId = default(-1); // specifies VLAN, unused by default
        int userPriority = default(-1); // specifies user priority, unused by default
        double transmissionPower @unit(W) = default(nan W); // specifies signal transmission power, unused by default
        @class(PacketTagger);
        @display("i=block/star");
    gates:
        input in;
        output out;
}
