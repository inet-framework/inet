//
// Copyright (C) 2006 Alfonso Ariza
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.applications.udpapp;


import inet.applications.IUDPApp;

//
// Sends \UDP packets to the given \IP address at the given interval.
// Compatible with both IPv4 and IPv6.
//
// The interval can be a constant or a random value (e.g. exponential(1)).
// If the dest_addresses parameter contains more than one address, one
// of them is randomly chosen for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The IPvXAddressResolver
// class is used to resolve the address.) To disable the model, set
// dest_addresses to "".
//
// The peer can be UDPSink or another UDPBasicApp (it handles received packets
// exactly like UDPSink).
//
// TODO: this is a copy-paste comment, revise !!!
// 
simple UDPBasicBurst like IUDPApp
{
    parameters:
        int localPort;
        int destPort;
        volatile int messageLength @unit("B"); //  length of messages to generate, in bytes
        volatile double burstDuration @unit("s");  // burst duration time
        volatile double sleepDuration @unit("s");  // time without transmision
        volatile double messageFreq @unit("s"); // should usually be a random value, e.g. exponential(1s)+uniform(-0.001s,0.001s)
        string destAddresses; // list of \IP addresses or node paths separated by spaces, if empty, the application is a sink.
                              // Use moduleListByPath(), moduleListByNedType() NED functions, if need
        int destAddrRNG = default(0);// random generator ID for select current dest address

        string chooseDestAddrMode @enum("once","perBurst","perSend"); // When change the destination address

        double startTime @unit("s");// begin transmission time
        double stopTime @unit("s"); // time of the last transmission packet 
        double limitDelay @unit("s"); // maximum delay for a packet, every packet with bigger delay is delete and lost ==>delayLimit
        @statistic[sentPk](title="packets sent"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[recvPk](title="packets received"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[duplPk](title="packets duplicated"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay";record=histogram,stats,vector);

    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

