//
// Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.ipv4;

import inet.common.SimpleModule;
import inet.networklayer.configurator.contract.IIpv4NodeConfigurator;

//
// ~HostAutoConfigurator automatically assigns IP addresses and sets up the routing table.
// It has to be added to each host.
//
// This configurator is deprecated. Please use ~Ipv4NetworkConfigurator whenever possible.
// This module is kept only because the ~Ipv4NetworkConfigurator cannot assign IP addresses
// to modules that are dynamically created/destroyed.
//
simple HostAutoConfigurator extends SimpleModule like IIpv4NodeConfigurator
{
    parameters:
        @class(HostAutoConfigurator);
        string interfaceTableModule;
        string interfaces = default("wlan");         // List of interfaces to autoassign addresses to, separated by a single space character
        string addressBase = default("10.0.0.0");    // Start of the address range from which to automatically assign an address to the autoassignInterfaces
        string netmask = default("255.0.0.0");       // Subnet netmask in which to automatically assign an address to the autoassignInterfaces
        string mcastGroups = default("");         // List of IP addresses of multicast groups to join, separated by a single space character
        @display("i=block/cogwheel");
}

