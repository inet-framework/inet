//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.generic;

//
// Defines a common interface for application modules that generate or consume
// traffic directly over IP, bypassing transport protocols like TCP or UDP.
// Provides standardized input and output gates with appropriate labels for
// connecting to the IP layer, supporting both IPv4 and IPv6 protocols through
// a unified interface.
//
// Implementations of this interface can create custom traffic patterns for
// network testing, performance evaluation, or protocol analysis without the
// overhead or constraints of transport layer protocols. The direct IP approach
// allows for precise control over packet characteristics and timing, making it
// particularly useful for low-level network testing and simulation scenarios.
//
// @see ~IpvxTrafGen, ~IpvxTrafSink, ~Ipv4, ~Ipv6
//
moduleinterface IIpvxTrafficGenerator
{
    @display("i=block/app");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}
