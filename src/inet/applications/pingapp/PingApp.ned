//
// Copyright (C) 2001 Monash University, Australia
// Copyright (C) 2012-2015 OpenSim Ltd.
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
// It works exactly like 'ping' except that it is possible to specify
// several destination addresses as a space separated list of IP addresses
// or module names. (The L3AddressResolver class is used to resolve the address.)
// Specifying '*' allows pinging ALL configured network interfaces in the
// whole simulation. This is useful to check if a host can reach ALL other
// hosts in the network (i.e. routing tables were set up properly).
//
// Start/stop time, sendInterval etc. can be specified via parameters. An address
// may be given in the dotted decimal notation (or, for IPv6, in the usual
// notation with colons), or with the module name.
// (The L3AddressResolver class is used to resolve the address.)

// To specify the number of ping requests sent to a single destination address,
// use the 'count' parameter. After the specified number of ping requests was
// sent to a destination address, the application goes to sleep for 'sleepDuration'.
// Once the sleep timer has expired, the application switches to the next destination
// and starts pinging again. The application stops pinging once all destination
// addresses were tested or the simulation time reaches 'stopTime'.

// To disable send, specify empty destAddr.
//
// Every ping request is sent out with a sequence number, and replies are
// expected to arrive in the same order. Whenever there's a jump in the
// in the received ping responses' sequence number (e.g. 1, 2, 3, 5), then
// the missing pings (number 4 in this example) is counted as lost.
// Then if it still arrives later (that is, a reply with a sequence number
// smaller than the largest one received so far) it will be counted as
// out-of-sequence arrival. So the number of really lost pings will be
// "lost" minus "out-of-order" (assuming there's no duplicate or bogus reply).
//
// Uses ~PingPayload as payload for the ICMP(v6) Echo Request/Reply packets.
//
// @see ~PingPayload, ~ICMP, ~ICMPv6
//
simple PingApp like IPingApp
{
    parameters:
        string destAddr = default(""); // destination address(es), separated by spaces, "*" means all IPv4/IPv6 interfaces in entire simulation
        string srcAddr = default(""); // source address (useful with multi-homing)
        double packetSize @unit("B") = default(56B); // of ping payload, in bytes
        volatile double sendInterval @unit("s") = default(1s); // time to wait between pings (can be random)
        double hopLimit = default(32); // TTL or hopLimit for IP packets
        double count = default(-1); // number of pings requests sent to a single destination address, -1 means continuously (only first address will be used from destAddr)
        double startTime @unit("s") = default(uniform(0s, this.sleepDuration + this.sendInterval)); // send first ping at startTime
        double stopTime @unit("s") = default(-1s); // time to finish sending, negative values mean forever
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
}

