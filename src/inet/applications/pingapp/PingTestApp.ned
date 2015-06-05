//
// Copyright (C) 2012 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.pingapp;

import inet.applications.contract.IPingApp;

//
// Generates ping requests to several hosts (or rather, network interfaces),
// and calculates the packet loss and round trip times of the replies.
// It works exactly like ~PingApp except that it is possible to specify
// several destination addresses as a space separated list of IP addresses
// or module names. Specifying '*' allows pinging ALL configured network
// interfaces in the whole simulation. This is useful to check if a host
// can reach ALL other hosts in the network (i.e. routing tables were set up
// properly).
//
// To specify the number of ping requests sent to a single destination address,
// use the 'count' parameter. After the specified number of ping requests was
// sent to a destination address, the application goes to sleep for 'sleepDuration'.
// Once the sleep timer has expired, the application switches to the next destination
// and starts pinging again. The application stops pinging once all destination
// addresses were tested or the simulation time reaches 'stopTime'.
//
// @see ~PingApp, ~PingPayload, ~ICMP, ~ICMPv6
//
simple PingTestApp extends PingApp
{
    parameters:
        @class(PingApp);
        string destAddresses = default(""); // destination addresses; "*" means all IPv4/IPv6 interfaces in entire simulation
        destAddr = destAddresses;
        count = default(1);     // number of pings requests sent to a single destination address (must be > 0)
        startTime = default(uniform(0s,this.sleepDuration)); // send first ping at startTime
        sleepDuration = default(0s); // time spent in sleep between switching destinations
}

