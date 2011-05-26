//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
package inet.util;

//
// Records PCAP traces of frames sent/received by other modules within
// the same host. By default, it records frames sent/received by L2 modules
// of ~StandardHost and ~Router. The output filename is expected in the
// pcapFile parameter. The PcapRecorder module can also print tcpdump-like
// textual information to on the log (EV); this functionality can be
// controlled by the verbose parameter.
//
// <b>Which modules to record:</b> The list of modules can to be specified in
// the moduleNamePatterns parameter. It is a space-separated list of module
// names, which will be interpreted as children of the PcapRecorder's parent
// module. To record elements of a module vector, add "[*]" to the name
// (example: "eth[*]").
//
// <b>Operation:</b> PcapRecorder adds signal listeners to the recorded
// modules. The signals subscribed to are "packetSentToLower" and
// "packetReceivedFromLower", but these names can be overridden with the
// sendingSignalNames and receivingSignalNames parameters. The packets
// themselves are expected as cPacket* signal values.
//
// <b>Supported packets:</b> The module handles IPv4/IPv6 packets (IPv4Datagram/
// IPv6Datagram classes), possibly encapsulated into other (L2, PHY, etc)
// frames at any depth. Packets not containing IP packets are discarded.
// The following higher layer protocol messages (inside IP packets) are
// recognized and dumped/recorded: IPv4Datagram, SCTPMessage, TCPSegment,
// ICMPMessage.
//
// <b>Bugs:</b> IPv6 datagrams cannot be recorded into PCAP. (To be implemented).
//
simple PcapRecorder {
    parameters:
        bool verbose = default(false);  // whether to log packets on the module output
        string pcapFile = default(""); // the PCAP file to be written
        int snaplen = default(65535);  // maximum number of bytes to record per packet
        bool dumpBadFrames = default(true); // enable dump of frames with hasBitError
        string moduleNamePatterns = default("wlan[*] eth[*] ppp[*]"); // space-separated list of sibling module names to listen on
        string sendingSignalNames = default("packetSentToLower"); // space-separated list of outbound packet signals to subscribe to
        string receivingSignalNames = default("packetReceivedFromLower"); // space-separated list of inbound packet signals to subscribe to
}

