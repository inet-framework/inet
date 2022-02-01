//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.ActivePacketSourceBase;
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
simple PcapFilePacketProducer extends ActivePacketSourceBase like IActivePacketSource
{
    parameters:
        string filename; // the PCAP file to be read
        packetNameFormat = default("Pcap-%p-%t-%n"); // determines the name of the packets read from the PCAP file
        @class(PcapFilePacketProducer);
    gates:
        output out @labels(push);
}
