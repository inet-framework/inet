//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// A passive packet sink which is pushed with packets by the
// connected packet producer. All pushed packets are written to a PCAP file.
//
simple PcapFilePacketConsumer extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        string fileFormat @enum("pcap", "pcapng") = default("pcap");
        string filename; // The PCAP file to be written
        int networkType; // The network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html (1=ethernet, 204=ppp, 105=IEEE 802.11, ...)
        int snaplen = default(65535);  // Maximum number of bytes to record per packet
        int timePrecision = default(6); // Time precision in recorded file. pcap supports only 6 (usec) or 9 (nanosec), pcapng supports more values (see 'if_tsresol' option in pcapng file format).
        bool outbound;  // Direction flag
        bool alwaysFlush = default(false); // Flush the PCAP file after each write to ensure that all packets are captured in case of a crash
        @class(PcapFilePacketConsumer);
    gates:
        input in @labels(push);
}

