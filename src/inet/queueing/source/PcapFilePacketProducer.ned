//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.ActivePacketSourceBase;
import inet.queueing.contract.IActivePacketSource;

//
// Generates network traffic by reading packets from a PCAP file and injecting
// them into a simulation. Allows realistic traffic patterns captured from real
// networks to be replayed in simulated environments.
//
// Key features:
// - Reads standard PCAP/PCAPNG files created by Wireshark, tcpdump, etc.
// - Preserves original packet contents, sizes, and protocols
// - Configurable packet naming with format directives
// - Actively pushes packets to connected modules
// - Maintains the IActivePacketSource interface for compatibility
//
// Operation mechanism:
// - Reads packets sequentially from the specified PCAP file
// - Creates simulation packets with the same content as the captured packets
// - Pushes these packets to the connected consumer module
// - Optionally applies custom naming based on packet attributes
//
// The packet naming can be customized using format directives to include
// information about the packet source, destination, protocol, length, etc.
//
// @see ~PcapFilePacketConsumer, ~PacketFlowPcapFileRecorder
//
simple PcapFilePacketProducer extends ActivePacketSourceBase like IActivePacketSource
{
    parameters:
        string filename; // The PCAP file to be read
        packetNameFormat = default("Pcap-%p-%t-%n"); // Determines the name of the packets read from the PCAP file
        @class(PcapFilePacketProducer);
    gates:
        output out @labels(push);
}
