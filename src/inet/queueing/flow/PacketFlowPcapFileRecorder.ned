//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Records network packets passing through a flow point to a PCAP or PCAPNG file
// for later analysis with external tools like Wireshark. Captures packets in
// standard formats compatible with most network analysis software.
//
// Key features:
// - Supports both PCAP and PCAPNG file formats
// - Configurable network type for proper protocol interpretation
// - Adjustable capture length (snaplen) to limit storage requirements
// - Configurable time precision for accurate timing analysis
// - Direction flag to indicate packet flow direction
// - Optional immediate file flushing for crash-resilient captures
//
// The recorder can be placed at any point in a network path where packet
// inspection is desired. Multiple recorders can be used simultaneously
// at different points to compare traffic patterns or analyze protocol
// transformations across network layers.
//
// Common network types include:
// - 1: Ethernet
// - 105: IEEE 802.11 wireless LAN
// - 204: PPP
// - 228: IPv4
// - 229: IPv6
//
// For a complete list of network types, see: http://www.tcpdump.org/linktypes.html
//
simple PacketFlowPcapFileRecorder extends PacketFlowBase like IPacketFlow
{
    parameters:
        string fileFormat @enum("pcap","pcapng") = default("pcap"); // Format of the capture file
        string filename; // The PCAP file to be written
        int networkType; // The network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html (1=ethernet, 204=ppp, 105=IEEE 802.11, ...)
        int snaplen = default(65535);  // Maximum number of bytes to record per packet
        int timePrecision = default(6); // Time precision in recorded file. pcap supports only 6 (usec) or 9 (nanosec), pcapng supports more values (see 'if_tsresol' option in pcapng file format)
        string direction @enum(inbound, outbound);  // Direction flag for packet flow
        bool alwaysFlush = default(false); // Flush the PCAP file after each write to ensure that all packets are captured in case of a crash
        @class(PacketFlowPcapFileRecorder);
}
