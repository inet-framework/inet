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

package inet.queueing.sink;

import inet.queueing.base.PacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is a passive packet sink which is pushed with packets by the
// connected packet producer. All pushed packets are written to a PCAP file.
//
simple PcapFilePacketConsumer extends PacketSinkBase like IPassivePacketSink
{
    parameters:
        string filename; // the PCAP file to be written
        int networkType; // the network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html (1=ethernet, 204=ppp, 105=IEEE 802.11, ...)
        int snaplen = default(65535);  // maximum number of bytes to record per packet
        bool alwaysFlush = default(false); // flush the PCAP file after each write to ensure that all packets are captured in case of a crash
        @class(PcapFilePacketConsumer);
        @signal[packetPushed](type=inet::Packet);
        @statistic[packetPushed](title="packets pushed"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input in @labels(push);
}

