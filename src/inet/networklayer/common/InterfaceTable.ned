//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.

package inet.networklayer.common;

//
// Keeps the table of network interfaces.
//
// Interfaces are dynamically registered by the corresponding L2 modules, e.g.
// ~PppInterface. In addition to registered interfaces, a loopback interface
// will also be created. This table only contains protocol-independent
// properties of interfaces -- IPv4 or IPv6 specific per-interface data
// (addresses etc) are kept in ~Ipv4RoutingTable and ~Ipv6RoutingTable.
// Must be named as "interfaceTable".
//
// This module has no gates; all functionality can be accessed via member
// functions of the C++ module class. For detailed info, please see the C++
// documentation of the class (Doxygen).
//
// @author: Andras Varga
//
simple InterfaceTable
{
    parameters:
        bool displayAddresses = default(false);  // whether to display IP addresses on links
        @display("i=block/table");
        @signal[interfaceCreated](type=inet::InterfaceEntry);
        @signal[interfaceDeleted](type=inet::InterfaceEntry);
        @signal[interfaceStateChanged](type=inet::InterfaceEntryChangeDetails);
        @signal[interfaceConfigChanged](type=inet::InterfaceEntryChangeDetails);
        @signal[interfaceIpv4ConfigChanged](type=inet::InterfaceEntryChangeDetails);
        @signal[interfaceIpv6ConfigChanged](type=inet::InterfaceEntryChangeDetails);
        @signal[interfaceGnpConfigChanged](type=inet::InterfaceEntryChangeDetails);
        @signal[ipv4MulticastGroupJoined](type=inet::Ipv4MulticastGroupInfo);
        @signal[ipv4MulticastGroupLeft](type=inet::Ipv4MulticastGroupInfo);
        @signal[ipv4McastChange](type=inet::Ipv4MulticastGroupSourceInfo);
        @signal[ipv4MulticastGroupRegistered](type=inet::Ipv4MulticastGroupInfo);
        @signal[ipv4MulticastGroupUnregistered](type=inet::Ipv4MulticastGroupInfo);
        @signal[ipv6MulticastGroupJoined](type=inet::Ipv6MulticastGroupInfo);
        @signal[ipv6MulticastGroupLeft](type=inet::Ipv6MulticastGroupInfo);
}

