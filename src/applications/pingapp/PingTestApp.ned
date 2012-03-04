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

import inet.applications.IPingApp;

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
simple PingTestApp like IPingApp
{
    parameters:
        string destAddresses = default(""); // destination addresses; "*" means all IPv4/IPv6 interfaces in entire simulation
        string srcAddr = default(""); // source address (useful with multi-homing)
        double packetSize @unit("B") = default(56B); // of ping payload, in bytes
        volatile double sendInterval @unit("s") = default(1s); // time to wait between pings (can be random)
        double hopLimit = default(32); // TTL or hopLimit for IP packets
        double count = default(1);     // number of pings requests sent to a single destination address (must be > 0)
        double startTime @unit("s") = default(uniform(0s,this.sleepDuration)); // send first ping at startTime
        double stopTime @unit("s") = default(0s); // time to finish sending, 0 means forever
        volatile double sleepDuration @unit(s) = default(0s); // time spent in sleep between switching destinations
        bool continuous = default(false);  // whether to continuously ping the destinations in a round-robin fashion
        bool printPing = default(false); // log to stdout
        @display("i=block/app");
        @signal[rtt](type=simtime_t);
        @signal[numLost](type=long);
        @signal[outOfOrderArrivals](type=long);
        @signal[pingTxSeq](type=long);
        @signal[pingRxSeq](type=long);
        @statistic[rtt](title="ping round-trip time"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numLost](title="pings lost"; record=last,vector; interpolationmode=none);
        @statistic[numOutOfOrderArrivals](title="ping out-of-order arrivals"; record=last,vector; interpolationmode=none);
        @statistic[pingTxSeq](title="ping tx seq"; record=count,vector; interpolationmode=none);
        @statistic[pingRxSeq](title="ping rx seq"; record=count,vector; interpolationmode=none);
    gates:
        input pingIn @labels(PingPayload/up);
        output pingOut @labels(PingPayload/down);
        input pingv6In @labels(PingPayload/up);
        output pingv6Out @labels(PingPayload/down);
}

