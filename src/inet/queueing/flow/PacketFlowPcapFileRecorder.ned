//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Records packets passing through the module to a PCAP or PCAPNG file. The file format,
// and fields like network type, time precision, and direction flag can be configured.
// It supports truncation of long packets. It can also be configured to flush the file
// after each write to ensure all packets are captured even in case of a crash.
//
simple PacketFlowPcapFileRecorder extends PacketFlowBase like IPacketFlow
{
    parameters:
        string fileFormat @enum("pcap","pcapng") = default("pcap");
        string filename; // the PCAP file to be written
        int networkType; // the network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html (1=ethernet, 204=ppp, 105=IEEE 802.11, ...)
        int snaplen = default(65535);  // maximum number of bytes to record per packet
        int timePrecision = default(6); // Time precison in recorded file. pcap supports only 6 (usec) or 9 (nanosec), pcapng supports more values (see 'if_tsresol' option in pcapng file format).
        string direction @enum(inbound, outbound);  // direction flag
        bool alwaysFlush = default(false); // flush the PCAP file after each write to ensure that all packets are captured in case of a crash
        @class(PacketFlowPcapFileRecorder);
}
