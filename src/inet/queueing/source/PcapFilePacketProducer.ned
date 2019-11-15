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

package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;

//
// This module is an active packet source which pushes packets into the connected
// packet consumer. All pushed packets are read from a PCAP file.
//
// Available packet name format directives:
//  %s - source address and/or port
//  %d - destination address and/or port
//  %p - innermost packet protocol
//  %t - innermost packet protocol subtype
//  %l - packet length
//  %i - packet data information
//  %n - number of packets read so far
//
simple PcapFilePacketProducer like IActivePacketSource
{
    parameters:
        string filename; // the PCAP file to be read
        string packetNameFormat = default("Pcap-%p-%t-%n"); // determines the name of the packets read from the PCAP file
        @class(PcapFilePacketProducer);
        @display("i=block/source");
        @signal[packetCreated](type=inet::Packet);
        @statistic[packetCreated](title="packets created"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        output out @labels(push);
}
