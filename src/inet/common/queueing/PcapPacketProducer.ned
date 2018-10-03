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

import inet.common.queueing.base.PacketSourceBase;
import inet.common.queueing.contract.IPacketProducer;

//
// This module is an active packet source which pushes packets into the connected
// packet consumer. All pushed packets are read from a PCAP file.
//
simple PcapPacketProducer like IPacketProducer
{
    parameters:
        string filename; // the PCAP file to be read
        string packetNameFormat = default("Pcap-%p-%t-%n");
        @class(PcapPacketProducer);
        @display("i=block/source");
    gates:
        output out @labels(push);
}
