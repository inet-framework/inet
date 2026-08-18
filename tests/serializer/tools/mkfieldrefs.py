#!/usr/bin/env python3
"""Write the field reference of a capture: what an independent decoder reads from it.

The two round-trip tests reproduce the bytes of a frame, so neither can see a serializer
that reads and writes a field in the same wrong place. serializer_fields.test can: it
compares the fields INET's dissection yields against the values in the reference file this
tool writes next to the capture.

The reference is a projection, not a tshark dump: the values are tshark's, the names are
INET's, and the mapping between them is the table below. That keeps the committed file in
the vocabulary of the test (so the test needs no mapping of its own and never runs
tshark), and keeps a Wireshark field rename in this tool rather than in the repository.

    tests/serializer/tools/mkfieldrefs.py pcap/bgp.pcap --frames 1
    tests/serializer/tools/mkfieldrefs.py pcap/generated/mrp-ring.pcap        # every frame

Run it from tests/serializer. The last column of every line names the tshark field the value
came from, because that pairing is what a reviewer has to check: a wrong pairing would
put a wrong value into the reference and the test would then confirm INET against it.
Fields with no mapping are counted in a comment per frame, so what is not covered stays
visible instead of silently missing.
"""

import argparse
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

# ---------------------------------------------------------------- value normalisers

def mac(value):
    """tshark prints a MAC lowercase with colons, MacAddress::str() uppercase with dashes."""
    return value.upper().replace(":", "-")

def hexint(value):
    """A field tshark shows in hex ("0x0008"), while the descriptor prints the number."""
    return str(int(value, 16)) if value.startswith("0x") else value

def hexstring(value):
    """A field both print in hex, the descriptor without the 0x (a transport checksum)."""
    return value[2:].lower() if value.startswith("0x") else value

def uint64(value):
    """The descriptor marks a 64-bit value with a suffix."""
    return hexint(value) + "u"

def ipv6(value):
    """INET names the unspecified address rather than printing it."""
    return "<unspec>" if value == "::" else value

def boolean(value):
    return "true" if value not in ("0", "False", "false") else "false"

def plain(value):
    return value

def ipv4(value):
    """INET names the unspecified address rather than printing it."""
    return "<unspec>" if value == "0.0.0.0" else value

def microseconds(value):
    """A field tshark counts in microseconds and the descriptor prints as a simtime, in
    the largest unit that leaves a value of at least one."""
    number = int(value)
    if number == 0:
        return "0s"
    if number < 1000:
        return "%dus" % number
    if number < 1000000:
        return ("%.6f" % (number / 1000.0)).rstrip("0").rstrip(".") + "ms"
    return ("%.9f" % (number / 1000000.0)).rstrip("0").rstrip(".") + "s"

def bytes_unit(value):
    """A length the model keeps in a B, which the descriptor prints with its unit."""
    return "%s B" % value

def enum_of(names):
    """A field declared with an enum type: the descriptor prints the number and the name
    it goes by ("6 (IP_PROT_TCP)"). The table is what INET calls the values, so a value a
    capture carries and INET has no name for is a finding, not something to paper over."""
    def convert(value):
        number = int(value, 0)
        if number not in names:
            raise ValueError("no INET name for enum value %d (%s)" % (number, value))
        return "%d (%s)" % (number, names[number])
    return convert

def four_seconds(value):
    """A lifetime the wire counts in units of four seconds (RFC 6275) and the model in
    seconds."""
    return str(int(value) * 4)

def seconds(value):
    """A field tshark counts in seconds and the descriptor prints as a simtime."""
    return microseconds(int(round(float(value) * 1000000)))

def port_identifier(part):
    """The port identifier of a BPDU is one 16-bit field to tshark, a priority and a port
    number here."""
    def convert(value):
        number = int(value, 0)
        return str(number >> 8 if part == "priority" else number & 0xff)
    return convert

def hexbytes(value):
    """A field tshark prints as octets ("00:00") and the descriptor as a number."""
    return str(int(value.replace(":", ""), 16))

def hexbytes_le(value):
    """Octets tshark prints in the order they sit on the wire, for a field the protocol
    writes little endian: the SCTP checksum, which is a CRC32c the sender byte-swaps."""
    return str(int.from_bytes(bytes.fromhex(value.replace(":", "")), "little"))

def from_value(convert):
    """Read the field's octets rather than its printed value. Wireshark prints some fields
    as a description of themselves -- a padding shows as "data" -- and the octets are the
    only thing left to compare."""
    def wrapper(value):
        return convert(value)
    wrapper.from_value = True
    return wrapper

def uuid_half(index):
    """The MRP domain uuid is one 128-bit field to tshark and two 64-bit fields here."""
    def convert(value):
        digits = value.replace("-", "")
        half = digits[:16] if index == 0 else digits[16:]
        return str(int(half, 16)) + "u"
    return convert

# The key under which a mapping entry names the field that decides which chunk class the
# value belongs to: ("parent", name) reads it from the field this one is nested in (the
# TLV of an MRP message), ("frame", name) from anywhere in the frame (the type of an
# ICMPv6 message).
DISCRIMINATOR = "#discriminator"

# tshark field prefixes --exclude leaves out; set from the command line
excluded = ()

# ---------------------------------------------------------------- the mapping
#
# tshark field -> (INET chunk class, INET field, normaliser). Add protocols as captures
# that carry them enter the corpus; what matters most is pinning fields whose neighbours
# are the same width (an address triplet, a port pair) and fields that have a reference of
# their own (a configured value, an address that also appears a layer down).

IP_PROTOCOL_NAMES = {
    1: "IP_PROT_ICMP", 2: "IP_PROT_IGMP", 6: "IP_PROT_TCP", 17: "IP_PROT_UDP",
    41: "IP_PROT_IPv6", 58: "IP_PROT_IPv6_ICMP", 88: "IP_PROT_EIGRP", 89: "IP_PROT_OSPF",
    103: "IP_PROT_PIM", 132: "IP_PROT_SCTP", 135: "IP_PROT_IPv6EXT_MOB",
}

ICMP_TYPE_NAMES = {
    0: "ICMP_ECHO_REPLY", 3: "ICMP_DESTINATION_UNREACHABLE", 5: "ICMP_REDIRECT",
    8: "ICMP_ECHO_REQUEST", 11: "ICMP_TIME_EXCEEDED", 12: "ICMP_PARAMETER_PROBLEM",
}

ICMPV6_TYPE_NAMES = {
    1: "ICMPv6_DESTINATION_UNREACHABLE", 2: "ICMPv6_PACKET_TOO_BIG", 3: "ICMPv6_TIME_EXCEEDED",
    4: "ICMPv6_PARAMETER_PROBLEM", 128: "ICMPv6_ECHO_REQUEST", 129: "ICMPv6_ECHO_REPLY",
    130: "ICMPv6_MLD_QUERY", 131: "ICMPv6_MLD_REPORT", 132: "ICMPv6_MLD_DONE",
    133: "ICMPv6_ROUTER_SOL", 134: "ICMPv6_ROUTER_AD", 135: "ICMPv6_NEIGHBOUR_SOL",
    136: "ICMPv6_NEIGHBOUR_AD", 143: "ICMPv6_MLDv2_REPORT",
}

ARP_OPCODE_NAMES = {
    1: "ARP_REQUEST", 2: "ARP_REPLY", 3: "ARP_RARP_REQUEST", 4: "ARP_RARP_REPLY",
}

WLAN_FRAME_TYPE_NAMES = {
    0x00: "ST_ASSOCIATIONREQUEST", 0x01: "ST_ASSOCIATIONRESPONSE", 0x02: "ST_REASSOCIATIONREQUEST",
    0x03: "ST_REASSOCIATIONRESPONSE", 0x04: "ST_PROBEREQUEST", 0x05: "ST_PROBERESPONSE",
    0x08: "ST_BEACON", 0x09: "ST_ATIM", 0x0a: "ST_DISASSOCIATION", 0x0b: "ST_AUTHENTICATION",
    0x0c: "ST_DEAUTHENTICATION", 0x0d: "ST_ACTION", 0x0e: "ST_NOACKACTION",
    0x18: "ST_BLOCKACK_REQ", 0x1a: "ST_PSPOLL", 0x1b: "ST_RTS", 0x1c: "ST_CTS", 0x1d: "ST_ACK",
    0x20: "ST_DATA", 0x28: "ST_DATA_WITH_QOS",
}

WLAN_ACK_POLICY_NAMES = {
    0: "NORMAL_ACK", 1: "NO_ACK", 2: "NO_EXPLICIT_ACK", 3: "BLOCK_ACK",
}

MIPV6_BE_STATUS_NAMES = {
    1: "UNKNOWN_BINDING_FOR_HOME_ADDRESS_DEST_OPTION", 2: "UNKNOWN_MH_TYPE",
}

MIPV6_TYPE_NAMES = {
    0: "BINDING_REFRESH_REQUEST", 1: "HOME_TEST_INIT", 2: "CARE_OF_TEST_INIT",
    3: "HOME_TEST", 4: "CARE_OF_TEST", 5: "BINDING_UPDATE", 6: "BINDING_ACKNOWLEDGEMENT",
    7: "BINDING_ERROR",
}

MRP_SUB_TLV_TYPE_NAMES = {
    0x00: "RESERVED", 0x01: "TEST_MGR_NACK", 0x02: "TEST_PROPAGATE", 0x03: "AUTOMGR",
}

MRP_OUI_NAMES = { 0x000000: "OUI", 0x00154e: "IEC", 0x080006: "SIEMENS" }

BPDU_PROTOCOL_NAMES = { 0: "SPANNING_TREE_PROTOCOL" }
BPDU_VERSION_NAMES = { 0: "SPANNING_TREE", 2: "RAPID_SPANNING_TREE", 3: "MULTIPLE_SPANNING_TREE" }
BPDU_TYPE_NAMES = { 0: "BPDU_CFG", 0x02: "BPDU_RAPID_OR_MULTIPLE_SPANNING_TREE", 0x80: "BPDU_TCN" }

MRP_TLV_TYPE_NAMES = {
    0x00: "END", 0x01: "COMMON", 0x02: "TEST", 0x03: "TOPOLOGYCHANGE", 0x04: "LINKDOWN",
    0x05: "LINKUP", 0x06: "INTEST", 0x07: "INTOPOLOGYCHANGE", 0x08: "INLINKDOWN",
    0x09: "INLINKUP", 0x0a: "INLINKSTATUSPOLL", 0x7f: "OPTION",
}

MAPPING = {
    # Ethernet. The frame's own length field and its FCS both have a reference of their
    # own: what the frame measures, and what its bytes add up to.
    "eth.src":              ("inet::EthernetMacHeader", "src", mac),
    "eth.dst":              ("inet::EthernetMacHeader", "dest", mac),
    "eth.type":             ("inet::EthernetMacHeader", "typeOrLength", hexint),
    "eth.len":              ("inet::EthernetMacHeader", "typeOrLength", plain),
    "eth.fcs":              ("inet::EthernetFcs", "fcs", hexint),

    # IEEE 802.2 LLC, and the SNAP header that extends it -- an expectation on the base
    # covers both, which is what the frames of the MRP and STP captures need
    "llc.dsap":             ("inet::Ieee8022LlcHeader", "dsap", hexint),
    "llc.ssap":             ("inet::Ieee8022LlcHeader", "ssap", hexint),
    "llc.control":          ("inet::Ieee8022LlcHeader", "control", hexint),
    "llc.oui":              ("inet::Ieee8022LlcSnapHeader", "oui", plain),
    "llc.type":             ("inet::Ieee8022LlcSnapHeader", "protocolId", hexint),

    # IPv4
    "ip.src":               ("inet::Ipv4Header", "srcAddress", ipv4),
    "ip.dst":               ("inet::Ipv4Header", "destAddress", ipv4),
    "ip.id":                ("inet::Ipv4Header", "identification", hexint),
    "ip.ttl":               ("inet::Ipv4Header", "timeToLive", plain),
    "ip.frag_offset":       ("inet::Ipv4Header", "fragmentOffset", plain),
    # tshark writes ip.version for an IPv6 frame too, so this one has to say which it is
    "ip.version": { DISCRIMINATOR: ("frame", "ip.version"),
        "4": ("inet::Ipv4Header", "version", plain) },
    "ip.hdr_len":           ("inet::Ipv4Header", "headerLength", bytes_unit),
    "ip.dsfield":           ("inet::Ipv4Header", "typeOfService", hexint),
    "ip.len":               ("inet::Ipv4Header", "totalLengthField", bytes_unit),
    "ip.flags.rb":          ("inet::Ipv4Header", "reservedBit", boolean),
    "ip.flags.df":          ("inet::Ipv4Header", "dontFragment", boolean),
    "ip.flags.mf":          ("inet::Ipv4Header", "moreFragments", boolean),
    "ip.proto":             ("inet::Ipv4Header", "protocolId", enum_of(IP_PROTOCOL_NAMES)),
    "ip.checksum":          ("inet::Ipv4Header", "checksum", hexint),

    # IPv6
    "ipv6.src":             ("inet::Ipv6Header", "srcAddress", ipv6),
    "ipv6.dst":             ("inet::Ipv6Header", "destAddress", ipv6),
    "ipv6.hlim":            ("inet::Ipv6Header", "hopLimit", plain),
    "ipv6.version":         ("inet::Ipv6Header", "version", plain),
    "ipv6.tclass":          ("inet::Ipv6Header", "trafficClass", hexint),
    "ipv6.flow":            ("inet::Ipv6Header", "flowLabel", hexint),
    "ipv6.plen":            ("inet::Ipv6Header", "payloadLength", bytes_unit),
    "ipv6.nxt":             ("inet::Ipv6Header", "protocolId", enum_of(IP_PROTOCOL_NAMES)),

    # TCP / UDP. The flag bits are neighbours of one width, which is the case this test
    # exists for: a round-trip cannot tell SYN from FIN if both directions agree.
    "tcp.srcport":          ("inet::tcp::TcpHeader", "srcPort", plain),
    "tcp.dstport":          ("inet::tcp::TcpHeader", "destPort", plain),
    "tcp.seq_raw":          ("inet::tcp::TcpHeader", "sequenceNo", plain),
    "tcp.ack_raw":          ("inet::tcp::TcpHeader", "ackNo", plain),
    "tcp.window_size_value": ("inet::tcp::TcpHeader", "window", plain),
    "tcp.hdr_len":          ("inet::tcp::TcpHeader", "headerLength", bytes_unit),
    "tcp.checksum":         ("inet::tcp::TcpHeader", "checksum", hexint),
    "tcp.urgent_pointer":   ("inet::tcp::TcpHeader", "urgentPointer", plain),
    "tcp.flags.cwr":        ("inet::tcp::TcpHeader", "cwrBit", boolean),
    "tcp.flags.ece":        ("inet::tcp::TcpHeader", "eceBit", boolean),
    "tcp.flags.urg":        ("inet::tcp::TcpHeader", "urgBit", boolean),
    "tcp.flags.ack":        ("inet::tcp::TcpHeader", "ackBit", boolean),
    "tcp.flags.push":       ("inet::tcp::TcpHeader", "pshBit", boolean),
    "tcp.flags.reset":      ("inet::tcp::TcpHeader", "rstBit", boolean),
    "tcp.flags.syn":        ("inet::tcp::TcpHeader", "synBit", boolean),
    "tcp.flags.fin":        ("inet::tcp::TcpHeader", "finBit", boolean),
    "udp.srcport":          ("inet::UdpHeader", "srcPort", plain),
    "udp.dstport":          ("inet::UdpHeader", "destPort", plain),
    "udp.length":           ("inet::UdpHeader", "totalLengthField", bytes_unit),
    "udp.checksum":         ("inet::UdpHeader", "checksum", hexstring),

    # ARP
    "arp.src.hw_mac":       ("inet::ArpPacket", "srcMacAddress", mac),
    "arp.dst.hw_mac":       ("inet::ArpPacket", "destMacAddress", mac),
    "arp.src.proto_ipv4":   ("inet::ArpPacket", "srcIpAddress", ipv4),
    "arp.dst.proto_ipv4":   ("inet::ArpPacket", "destIpAddress", ipv4),
    "arp.opcode":           ("inet::ArpPacket", "opcode", enum_of(ARP_OPCODE_NAMES)),

    # ICMP and ICMPv6: the type is what tells the message classes apart, so an expectation
    # on the base class holds for whichever one the frame turns out to carry
    "icmp.type":            ("inet::IcmpHeader", "type", enum_of(ICMP_TYPE_NAMES)),
    "icmp.code":            ("inet::IcmpHeader", "code", plain),
    "icmp.checksum":        ("inet::IcmpHeader", "chksum", hexint),
    # the identifier and the sequence number are declared in the echo classes, one per
    # direction, and the type is what says which of them the frame carries
    "icmp.ident": { DISCRIMINATOR: ("frame", "icmp.type"),
        "8": ("inet::IcmpEchoRequest", "identifier", plain),
        "0": ("inet::IcmpEchoReply", "identifier", plain) },
    "icmp.seq": { DISCRIMINATOR: ("frame", "icmp.type"),
        "8": ("inet::IcmpEchoRequest", "seqNumber", plain),
        "0": ("inet::IcmpEchoReply", "seqNumber", plain) },
    "icmpv6.type":          ("inet::Icmpv6Header", "type", enum_of(ICMPV6_TYPE_NAMES)),
    "icmpv6.code":          ("inet::Icmpv6Header", "code", plain),
    "icmpv6.checksum":      ("inet::Icmpv6Header", "chksum", hexint),

    # IEEE 802.11 -- the three addresses are the classic swap candidates. Take them from
    # the positional fields: what tshark calls ra/ta/bssid/da/sa is which address means
    # what given ToDS and FromDS, while the model names them by where they sit. They live
    # in base classes that the data and the management header both extend, and the test
    # matches a base class as well, so one expectation covers either kind of frame.
    "wlan.addr1":           ("inet::ieee80211::Ieee80211OneAddressHeader", "receiverAddress", mac),
    "wlan.addr2":           ("inet::ieee80211::Ieee80211TwoAddressHeader", "transmitterAddress", mac),
    "wlan.addr3":           ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "address3", mac),
    # what a data frame's addresses are named once ToDS and FromDS have been read: the same
    # three positions, so the same three fields
    "wlan.ra":              ("inet::ieee80211::Ieee80211OneAddressHeader", "receiverAddress", mac),
    "wlan.ta":              ("inet::ieee80211::Ieee80211TwoAddressHeader", "transmitterAddress", mac),
    # what the third address means is what the two DS bits say, and tshark names it
    # accordingly -- the model keeps it by position, so the DS bits pick the name to read
    "wlan.bssid": { DISCRIMINATOR: ("frame", "wlan.fc.ds"),
        "0x00": ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "address3", mac) },
    "wlan.da": { DISCRIMINATOR: ("frame", "wlan.fc.ds"),
        "0x01": ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "address3", mac) },
    "wlan.sa": { DISCRIMINATOR: ("frame", "wlan.fc.ds"),
        "0x02": ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "address3", mac) },
    "wlan.frag":            ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "fragmentNumber", plain),
    "wlan.seq":             ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "sequenceNumber", plain),
    "wlan.duration":        ("inet::ieee80211::Ieee80211MacHeader", "durationField", microseconds),
    "wlan.fc.type_subtype": ("inet::ieee80211::Ieee80211MacHeader", "type", enum_of(WLAN_FRAME_TYPE_NAMES)),
    "wlan.fc.tods":         ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "toDS", boolean),
    "wlan.fc.fromds":       ("inet::ieee80211::Ieee80211DataOrMgmtHeader", "fromDS", boolean),
    "wlan.fc.frag":         ("inet::ieee80211::Ieee80211MacHeader", "moreFragments", boolean),
    "wlan.fc.retry":        ("inet::ieee80211::Ieee80211MacHeader", "retry", boolean),
    "wlan.fc.pwrmgt":       ("inet::ieee80211::Ieee80211MacHeader", "powerMgmt", boolean),
    "wlan.fc.moredata":     ("inet::ieee80211::Ieee80211MacHeader", "moreData", boolean),
    "wlan.fc.protected":    ("inet::ieee80211::Ieee80211MacHeader", "protectedFrame", boolean),
    "wlan.fc.order":        ("inet::ieee80211::Ieee80211MacHeader", "order", boolean),
    "wlan.fcs":             ("inet::ieee80211::Ieee80211MacTrailer", "fcs", hexint),
    # the QoS control octet: four fields packed into one byte, which is exactly the shape
    # a round-trip cannot check
    "wlan.qos.tid":         ("inet::ieee80211::Ieee80211DataHeader", "tid", plain),
    "wlan.qos.eosp":        ("inet::ieee80211::Ieee80211DataHeader", "eosp", boolean),
    "wlan.qos.ack":         ("inet::ieee80211::Ieee80211DataHeader", "ackPolicy", enum_of(WLAN_ACK_POLICY_NAMES)),
    "wlan.qos.amsdupresent": ("inet::ieee80211::Ieee80211DataHeader", "aMsduPresent", boolean),

    # VLAN / 802.1Q, and the 802.1CB tag in front of it. A stacked frame has one chunk per
    # tag, and the values arrive outer first, which is the order the occurrence counts in.
    "ieee8021ad.id":        ("inet::Ieee8021qTagEpdHeader", "vid", plain),
    "ieee8021ad.priority":  ("inet::Ieee8021qTagEpdHeader", "pcp", plain),
    "ieee8021ad.dei":       ("inet::Ieee8021qTagEpdHeader", "dei", boolean),
    "ieee8021ah.etype":     ("inet::Ieee8021qTagEpdHeader", "typeOrLength", hexint),
    "vlan.id":              ("inet::Ieee8021qTagEpdHeader", "vid", plain),
    "vlan.priority":        ("inet::Ieee8021qTagEpdHeader", "pcp", plain),
    "vlan.dei":             ("inet::Ieee8021qTagEpdHeader", "dei", boolean),
    "vlan.etype":           ("inet::Ieee8021qTagEpdHeader", "typeOrLength", hexint),

    # MPLS
    "mpls.label":           ("inet::MplsHeader", "label", plain),
    "mpls.ttl":             ("inet::MplsHeader", "ttl", plain),
    "mpls.bottom":          ("inet::MplsHeader", "s", boolean),
    "mpls.exp":             ("inet::MplsHeader", "tc", plain),

    # STP. The configuration BPDU and the topology change notification share a base, and
    # the three fields they have in common are declared there.
    "stp.protocol":         ("inet::BpduBase", "protocolIdentifier", enum_of(BPDU_PROTOCOL_NAMES)),
    "stp.version":          ("inet::BpduBase", "protocolVersionIdentifier", enum_of(BPDU_VERSION_NAMES)),
    "stp.type":             ("inet::BpduBase", "bpduType", enum_of(BPDU_TYPE_NAMES)),
    "stp.port":             [("inet::BpduCfg", "portPriority", port_identifier("priority")),
                             ("inet::BpduCfg", "portNum", port_identifier("number"))],
    "stp.msg_age":          ("inet::BpduCfg", "messageAge", seconds),
    "stp.max_age":          ("inet::BpduCfg", "maxAge", seconds),
    "stp.hello":            ("inet::BpduCfg", "helloTime", seconds),
    "stp.forward":          ("inet::BpduCfg", "forwardDelay", seconds),
    # tshark splits the priority half of a bridge identifier into a priority and the system
    # id extension below it, while the model keeps the two as one number -- but the octets
    # behind either half are the whole field, so read those
    "stp.root.ext":         ("inet::BpduCfg", "rootPriority", from_value(hexbytes)),
    "stp.bridge.ext":       ("inet::BpduCfg", "bridgePriority", from_value(hexbytes)),
    "stp.root.hw":          ("inet::BpduCfg", "rootAddress", mac),
    "stp.root.cost":        ("inet::BpduCfg", "rootPathCost", plain),
    "stp.bridge.hw":        ("inet::BpduCfg", "bridgeAddress", mac),

    "stp.flags.tc":         ("inet::BpduCfg", "tcFlag", boolean),
    "stp.flags.tcack":      ("inet::BpduCfg", "tcaFlag", boolean),

    # PPP
    "ppp.address":          ("inet::PppHeader", "address", hexint),
    "ppp.control":          ("inet::PppHeader", "control", hexint),
    "ppp.protocol":         ("inet::PppHeader", "protocol", hexint),

    # RTP
    "rtp.ssrc":             ("inet::rtp::RtpHeader", "ssrc", hexint),
    "rtp.seq":              ("inet::rtp::RtpHeader", "sequenceNumber", plain),
    "rtp.timestamp":        ("inet::rtp::RtpHeader", "timeStamp", plain),
    "rtp.p_type":           ("inet::rtp::RtpHeader", "payloadType", plain),

    # SCTP
    "sctp.srcport":         ("inet::sctp::SctpHeader", "srcPort", plain),
    "sctp.dstport":         ("inet::sctp::SctpHeader", "destPort", plain),
    "sctp.verification_tag": ("inet::sctp::SctpHeader", "vTag", hexint),
    # tshark reads the checksum octets as one big endian number; the field holds the CRC32c
    # the sender computed, which goes on the wire in little endian order
    "sctp.checksum":        ("inet::sctp::SctpHeader", "checksum", from_value(hexbytes_le)),
    # the chunks of an SCTP packet are objects inside the header rather than chunks of
    # the packet, so the dissection does not yield them and there is nothing here to
    # compare them against -- reaching them would need a path into a field of a field

    # MLD: the response delay and the reserved octets are declared in the base every
    # message extends, so one expectation holds whichever kind the frame carries. A
    # reserved field is worth having: it is where a serializer that has slipped by a field
    # shows something other than the zero it must write.
    "icmpv6.mld.maximum_response_delay": ("inet::MldMessage", "maxRespDelay", plain),
    "icmpv6.reserved": { DISCRIMINATOR: ("frame", "icmpv6.type"),
        "130": ("inet::MldMessage", "reserved", hexbytes),
        "131": ("inet::MldMessage", "reserved", hexbytes),
        "132": ("inet::MldMessage", "reserved", hexbytes),
        "143": ("inet::Mldv2Report", "reserved", hexbytes) },
    "icmpv6.mld.multicast_address":      ("inet::MldMessage", "multicastAddress", ipv6),
    # MLDv2 packs the query's parameters into one octet and a code: the robustness variable
    # and the suppress flag sit in it, the query interval follows
    "icmpv6.mld.maximum_response_code":  ("inet::MldMessage", "maxRespDelay", plain),
    "icmpv6.mld.qqi":                    ("inet::Mldv2Query", "queryIntervalCode", plain),
    "icmpv6.mld.flag.qrv":               ("inet::Mldv2Query", "robustnessVariable", plain),
    "icmpv6.mld.flag.reserved":          ("inet::Mldv2Query", "resv", plain),
    "icmpv6.mld.flag.s":                 ("inet::Mldv2Query", "suppressRouterProc", boolean),

    # IEEE 802.1CB R-TAG
    "rtag.seqno":           ("inet::Ieee8021rTagEpdHeader", "sequenceNumber", plain),
    "rtag.protocol":        ("inet::Ieee8021rTagEpdHeader", "typeOrLength", hexint),

    # MIPv6 mobility header: one class per message type, and the type field names it
    "mip6.mhtype":          ("inet::MobilityHeader", "mobilityHeaderType", enum_of(MIPV6_TYPE_NAMES)),
    "mip6.bu.seqnr":        ("inet::BindingUpdate", "sequence", plain),
    "mip6.bu.lifetime":     ("inet::BindingUpdate", "lifetime", four_seconds),
    "mip6.bu.a_flag":       ("inet::BindingUpdate", "ackFlag", boolean),
    "mip6.bu.h_flag":       ("inet::BindingUpdate", "homeRegistrationFlag", boolean),
    "mip6.bu.l_flag":       ("inet::BindingUpdate", "linkLocalAddressCompatibilityFlag", boolean),
    "mip6.bu.k_flag":       ("inet::BindingUpdate", "keyManagementFlag", boolean),
    "mip6.bu.p_flag":       ("inet::BindingUpdate", "proxyRegistrationFlag", boolean),
    "mip6.be.status":       ("inet::BindingError", "status", enum_of(MIPV6_BE_STATUS_NAMES)),
    "mip6.be.haddr":        ("inet::BindingError", "homeAddress", ipv6),
    "mip6.hoti.cookie":     ("inet::HomeTestInit", "homeInitCookie", uint64),
    "mip6.coti.cookie":     ("inet::CareOfTestInit", "careOfInitCookie", uint64),
    "mip6.hot.cookie":      ("inet::HomeTest", "homeInitCookie", uint64),
    # Wireshark gives the keygen token of a care-of test the home test's field name, so
    # which message it is has to come from the type
    "mip6.hot.token": { DISCRIMINATOR: ("frame", "mip6.mhtype"),
        "3": ("inet::HomeTest", "homeKeyGenToken", uint64),
        "4": ("inet::CareOfTest", "careOfKeyGenToken", uint64) },
    "mip6.hot.nindex":      ("inet::HomeTest", "homeNonceIndex", plain),
    "mip6.cot.cookie":      ("inet::CareOfTest", "careOfInitCookie", uint64),
    "mip6.cot.nindex":      ("inet::CareOfTest", "careOfNonceIndex", plain),

    # CFM: the MAID names the maintenance domain and the association inside it, each with
    # the format it is written in -- the field this model reads one name from
    "cfm.md_level":         ("inet::CfmContinuityCheckMessage", "mdLevel", plain),
    "cfm.opcode":           ("inet::CfmContinuityCheckMessage", "opCode", plain),
    "cfm.flags":            ("inet::CfmContinuityCheckMessage", "flags", hexint),
    "cfm.ccm.seq_num":      ("inet::CfmContinuityCheckMessage", "sequenceNumber", plain),
    "cfm.mep_id":           ("inet::CfmContinuityCheckMessage", "endpointIdentifier", plain),
    "cfm.maid.md_name.format": ("inet::CfmContinuityCheckMessage", "mdNameFormat", plain),
    "cfm.maid.md_name.string": ("inet::CfmContinuityCheckMessage", "mdName", plain),
    "cfm.maid.ma_name.format": ("inet::CfmContinuityCheckMessage", "maNameFormat", plain),
    "cfm.maid.ma_name.string": ("inet::CfmContinuityCheckMessage", "maName", plain),
    "cfm.version":          ("inet::CfmMessage", "version", plain),
    # the TLVs that follow: this model reads any of them as a raw one, and the type octet
    # is what says where the list ends
    "cfm.tlv.type":         ("inet::CfmTlvBase", "type", plain),
    "cfm.tlv.length":       ("inet::CfmTlvRaw", "length", plain),

    # MRP: every message is a TLV, and the TLV it sits in says which class holds it. The
    # nesting of the decode gives that away: these fields are children of the type field.
    "pn_mrp.sa": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "sa", mac),
        "0x03": ("inet::MrpTopologyChange", "sa", mac),
        "0x04": ("inet::MrpLinkChange", "sa", mac),
        "0x05": ("inet::MrpLinkChange", "sa", mac),
        "0x06": ("inet::MrpInTest", "sa", mac),
        "0x07": ("inet::MrpInTopologyChange", "sa", mac),
        "0x08": ("inet::MrpInLinkChange", "sa", mac),
        "0x09": ("inet::MrpInLinkChange", "sa", mac),
        "0x0a": ("inet::MrpInLinkStatusPoll", "sa", mac),
        "0x7f": ("inet::MrpSubTlvTest", "sa", mac) },
    "pn_mrp.prio": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "prio", hexint),
        "0x03": ("inet::MrpTopologyChange", "prio", hexint),
        "0x7f": ("inet::MrpSubTlvTest", "prio", hexint) },
    "pn_mrp.port_role": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "portRole", hexint),
        "0x06": ("inet::MrpInTest", "portRole", hexint),
        "0x04": ("inet::MrpLinkChange", "portRole", hexint),
        "0x05": ("inet::MrpLinkChange", "portRole", hexint),
        "0x08": ("inet::MrpInLinkChange", "portRole", hexint),
        "0x09": ("inet::MrpInLinkChange", "portRole", hexint),
        "0x0a": ("inet::MrpInLinkStatusPoll", "portRole", hexint) },
    "pn_mrp.ring_state": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "ringState", hexint) },
    "pn_mrp.transition": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "transition", hexint),
        "0x06": ("inet::MrpInTest", "transition", hexint) },
    "pn_mrp.inid": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x06": ("inet::MrpInTest", "inID", hexint),
        "0x07": ("inet::MrpInTopologyChange", "inID", hexint),
        "0x08": ("inet::MrpInLinkChange", "inID", hexint),
        "0x09": ("inet::MrpInLinkChange", "inID", hexint),
        "0x0a": ("inet::MrpInLinkStatusPoll", "inID", hexint) },
    "pn_mrp.interval": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x03": ("inet::MrpTopologyChange", "interval", hexint),
        "0x04": ("inet::MrpLinkChange", "interval", hexint),
        "0x05": ("inet::MrpLinkChange", "interval", hexint),
        "0x07": ("inet::MrpInTopologyChange", "interval", hexint),
        "0x08": ("inet::MrpInLinkChange", "interval", hexint),
        "0x09": ("inet::MrpInLinkChange", "interval", hexint) },
    "pn.padding": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x04": ("inet::MrpLinkChange", "reserved", from_value(hexbytes)),
        "0x05": ("inet::MrpLinkChange", "reserved", from_value(hexbytes)) },
    "pn_mrp.blocked": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x04": ("inet::MrpLinkChange", "blocked", hexint),
        "0x05": ("inet::MrpLinkChange", "blocked", hexint) },
    # what every MRP TLV carries: the type that says which class it is, and the length of
    # what follows it. Both are declared in the base the message classes extend.
    "pn_mrp.type":          ("inet::MrpTlvHeader", "headerType", enum_of(MRP_TLV_TYPE_NAMES)),
    "pn_mrp.length":        ("inet::MrpTlvHeader", "valueLength", plain),
    "pn_mrp.version":       ("inet::MrpVersion", "version", plain),
    "pn_mrp.time_stamp": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x02": ("inet::MrpTest", "timeStamp", hexint),
        "0x06": ("inet::MrpInTest", "timeStamp", hexint) },
    "pn_mrp.in_state": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x06": ("inet::MrpInTest", "inState", hexint) },
    "pn_mrp.linkinfo": { DISCRIMINATOR: ("parent", "pn_mrp.type"),
        "0x08": ("inet::MrpInLinkChange", "linkInfo", hexint),
        "0x09": ("inet::MrpInLinkChange", "linkInfo", hexint) },
    # the option TLV that carries them, and the sub-TLV an auto-manager sends: these
    # fields exist nowhere else
    "pn_mrp.oui":           ("inet::MrpOption", "ouiType", enum_of(MRP_OUI_NAMES)),
    "pn_mrp.ed1type":       ("inet::MrpOption", "ed1Type", hexint),
    "pn_mrp.sub_type":      ("inet::MrpSubTlvHeader", "subType", enum_of(MRP_SUB_TLV_TYPE_NAMES)),
    "pn_mrp.sub_length":    ("inet::MrpSubTlvHeader", "subHeaderLength", plain),
    "pn_mrp.other_mrm_prio": ("inet::MrpSubTlvTest", "otherMRMPrio", hexint),
    "pn_mrp.other_mrm_sa":  ("inet::MrpSubTlvTest", "otherMRMSa", mac),
    "pn_mrp.sequence_id":   ("inet::MrpCommon", "sequenceID", hexint),
    "pn_mrp.domain_uuid":   [("inet::MrpCommon", "uuid0", uuid_half(0)),
                             ("inet::MrpCommon", "uuid1", uuid_half(1))],
}

# ---------------------------------------------------------------- the work

def tshark_version():
    out = subprocess.run(["tshark", "-v"], capture_output=True, text=True).stdout
    return out.split("\n")[0].strip()

def read_pdml(filename, frames):
    args = ["tshark", "-r", filename, "-T", "pdml"]
    if frames:
        args += ["-Y", " || ".join("frame.number==%d" % f for f in frames)]
    result = subprocess.run(args, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit("tshark failed on %s: %s" % (filename, result.stderr.strip()))
    return ET.fromstring(result.stdout)

def frame_number(packet):
    for proto in packet.iter("proto"):
        if proto.get("name") == "geninfo":
            for field in proto.iter("field"):
                if field.get("name") == "num":
                    return int(field.get("show"))
    return None

def frame_values(packet):
    """Every field of the frame by name, for entries discriminated on the frame."""
    values = {}
    for field in packet.iter("field"):
        if field.get("name") is not None and field.get("show") is not None:
            values.setdefault(field.get("name"), field.get("show"))
    return values

def resolve(entry, name, values, parent):
    """Pick the mapping that applies here, or say why none does."""
    if isinstance(entry, dict):
        source, key = entry[DISCRIMINATOR]
        value = (parent.get(key) if source == "parent" else values.get(key))
        if value is None:
            return [], "%s: no %s to tell which chunk it belongs to" % (name, key)
        chosen = entry.get(value.lower())
        if chosen is None:
            return [], "%s with %s=%s" % (name, key, value)
        return [chosen], None
    return (entry if isinstance(entry, list) else [entry]), None

def walk(element, values, parent, rows, unmapped):
    for field in element.findall("field"):
        name, show = field.get("name"), field.get("show")
        # A field that names a subtree carries its own value again inside it -- the type
        # octet of an MRP TLV is both the node the TLV hangs under and a field of it. Take
        # the outer one and skip the repeat, or every such value would be counted twice
        # and push the occurrences of everything after it out of step.
        if name is not None and name in parent:
            walk(field, values, parent, rows, unmapped)
            continue
        if name is not None and show is not None and not any(name.startswith(p) for p in excluded):
            entry = MAPPING.get(name)
            if entry is None:
                if "." in name and not name.startswith("geninfo"):
                    unmapped.append(name)
            else:
                chosen, reason = resolve(entry, name, values, parent)
                for chunk, member, convert in chosen:
                    text = field.get("value") if getattr(convert, "from_value", False) else show
                    if text is not None:
                        rows.append((chunk, member, convert(text), name))
                if reason is not None:
                    unmapped.append(reason)
        # the fields of a TLV are nested in the field that names its type
        walk(field, values, ({ **parent, name: show } if name is not None else parent), rows, unmapped)
    # a decode that continues inside another one is a proto element inside a proto element,
    # not a field: the mobility header of an IPv6 packet sits under the IPv6 protocol. Miss
    # this and a whole protocol is invisible -- not even counted as unmapped.
    for sub in element.findall("proto"):
        walk(sub, values, parent, rows, unmapped)

def rows_of_frame(packet):
    """Every mapped field of one frame, in the order tshark decoded them."""
    rows, unmapped = [], []
    values = frame_values(packet)
    for proto in packet.findall("proto"):
        walk(proto, values, {}, rows, unmapped)
    return rows, unmapped

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("pcap", help="capture to read, e.g. pcap/generated/mrp-ring.pcap")
    parser.add_argument("--frames", help="comma-separated frame numbers; every frame by default")
    parser.add_argument("--exclude", default="", help="comma-separated tshark field prefixes to "
                        "leave out, for what Wireshark decodes deeper than INET dissects "
                        "(the LLC/SNAP header inside an 802.11 data frame, say): an "
                        "expectation about a chunk the dissection never yields could only fail")
    args = parser.parse_args()

    global excluded
    excluded = tuple(p for p in args.exclude.split(",") if p)
    frames = [int(f) for f in args.frames.split(",")] if args.frames else None
    root = read_pdml(args.pcap, frames)

    lines = ["# Field reference for %s: what tshark reads from it, named as INET names it." % os.path.basename(args.pcap),
             "# Generated by tests/serializer/tools/mkfieldrefs.py with %s -- do not edit by hand." % tshark_version(),
             "# columns: frame, chunk class, which chunk of that class, field, expected value,",
             "#          and the tshark field the value came from"]
    if excluded:
        lines.append("# left out on purpose (INET does not dissect this far): %s" % ", ".join(excluded))
    total = 0
    for packet in root.iter("packet"):
        number = frame_number(packet)
        rows, unmapped = rows_of_frame(packet)
        if not rows:
            continue
        occurrences = {}
        for chunk, member, value, source in rows:
            # a frame may hold several chunks of the same class -- a stack of tags or of
            # labels -- and the values arrive in the order they sit in the frame
            index = occurrences.get((chunk, member), 0)
            occurrences[(chunk, member)] = index + 1
            lines.append("%d\t%s\t%d\t%s\t%s\t%s" % (number, chunk, index, member, value, source))
            total += 1
        if unmapped:
            lines.append("# frame %d: %d fields of this frame have no mapping (%s%s)"
                         % (number, len(unmapped), ", ".join(sorted(set(unmapped))[:6]),
                            ", ..." if len(set(unmapped)) > 6 else ""))

    output = os.path.splitext(args.pcap)[0] + ".fields"
    with open(output, "w") as file:
        file.write("\n".join(lines) + "\n")
    print("%s: %d expectations" % (output, total))

if __name__ == "__main__":
    main()
