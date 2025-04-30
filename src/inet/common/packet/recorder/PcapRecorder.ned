//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.common.packet.recorder;

//
// Records PCAP traces of frames sent/received by other modules within
// the same host. By default, it records frames sent/received by L2 modules
// of ~StandardHost and ~Router. The output filename is expected in the
// `pcapFile` parameter. The `PcapRecorder` module can also print tcpdump-like
// textual information on the log (EV); this functionality can be
// controlled by the verbose parameter.
//
// <b>Which modules to record:</b> The list of modules can be specified in
// the `moduleNamePatterns` parameter. It is a space-separated list of module
// names, which will be interpreted as children of the `PcapRecorder`'s parent
// module. To record elements of a module vector, add "[*]" to the name
// (example: "eth[*]").
//
// <b>Operation:</b> `PcapRecorder` adds signal listeners to the recorded
// modules. The signals subscribed to are `packetSentToLower` and
// `packetReceivedFromLower`, but these names can be overridden with the
// `sendingSignalNames` and `receivingSignalNames` parameters. The packets
// themselves are expected as `cPacket*` signal values.
//
simple PcapRecorder
{
    parameters:
        bool verbose = default(true);  // Whether to log packets on the module output
        bool recordEmptyPackets = default(false); // Specifies if zero length packets are recorded or not
        bool enableConvertingPackets = default(true); // Specifies if converting packets to link type is allowed or not
        string pcapFile = default(""); // The PCAP file to be written
        string fileFormat @enum("pcap", "pcapng") = default("pcapng");
        int snaplen = default(65535);  // Maximum number of bytes to record per packet
        int timePrecision = default(6); // Time precision in recorded file. pcap supports only 6 (usec) or 9 (nanosec), pcapng supports more values (see 'if_tsresol' option in pcapng file format).
        bool dumpBadFrames = default(true); // Enable dump of frames with hasBitError
        string moduleNamePatterns = default("wlan[*] eth[*] ppp[*]"); // Space-separated list of sibling module names to listen on
        string sendingSignalNames = default("transmissionEnded"); // Space-separated list of outbound packet signals to subscribe to
        string receivingSignalNames = default("receptionEnded"); // Space-separated list of inbound packet signals to subscribe to
        string dumpProtocols = default("ethernetmac ppp ieee80211mac"); // Space-separated list of protocol names as defined in the Protocol class
        object packetFilter = default("*"); // Which packets are considered, matches all packets by default
        string helpers = default("");    // Usable PcapRecorder::IHelper helpers for accept packettype and store/convert packet as specified linktype
                                         // currently available: "inet::AckingMacToEthernetPcapRecorderHelper"
        bool alwaysFlush = default(false); // Flush the pcapFile after each write to ensure that all packets are captured in case of a crash
        string displayStringTextFormat = default("rec: %n pks");
        @display("i=block/blackboard");
        @signal[packetRecorded](type=Packet);
}

