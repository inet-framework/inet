//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is a passive packet sink which is pushed with packets by the
// connected packet producer. All pushed packets are written to a PCAP file.
//
simple PcapFilePacketConsumer extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        string filename; // the PCAP file to be written
        int networkType; // the network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html (1=ethernet, 204=ppp, 105=IEEE 802.11, ...)
        int snaplen = default(65535);  // maximum number of bytes to record per packet
        bool outbound;  // direction flag
        bool alwaysFlush = default(false); // flush the PCAP file after each write to ensure that all packets are captured in case of a crash
        @class(PcapFilePacketConsumer);
    gates:
        input in @labels(push);
}

