//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.generic;

//
// Module interface for modules that generate traffic directly over IP.
// Compatible with both ~Ipv4 and ~Ipv6.
//
// @see ~IpvxTrafGen, ~IpvxTrafSink
//
moduleinterface IIpvxTrafficGenerator
{
    @display("i=block/app");
    gates:
        input ipIn @labels(Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

