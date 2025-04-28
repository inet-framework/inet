//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.generic;

import inet.common.SimpleModule;

//
// Consumes and prints packets received from the IP module.
// Compatible with both ~Ipv4 and ~Ipv6.
//
simple IpvxTrafSink extends SimpleModule like IIpvxTrafficGenerator
{
    parameters:
        @class(IpvxTrafSink);
        int protocol; // Value for IPv4, IPv6 protocol field, accepts only values between 143 and 254
        displayStringTextFormat @mutable = default("rcvd: {numReceived} pks");
        @lifecycleSupport;
        @display("i=block/sink");
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

