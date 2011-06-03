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
// Sends UDP packets to the given IP address(es) in bursts, or acts as a
// packet sink. Compatible with both IPv4 and IPv6.
//
// <b>Addressing:</b>
//
// The destAddresses parameter can contain zero, one or more destination 
// addresses, separated by spaces. If there is no destination address given,
// the module will act as packet sink. If there are more than one addresses, 
// one of them is randomly chosen, either for the whole simulation run,
// or for each burst, or for each packet, depending on the value of the
// chooseDestAddrMode parameter. The destAddrRNG parameter controls which 
// (local) RNG is used for randomized address selection. 
//
// TODO: "broadcast" string mint address -- leirni!!!
// TODO: sink uzemmod -- ha kuld, akkor nem is tud fogadni??
// TODO: mit jelent a localPort == -1 ??
// 
// An address may be given in the dotted decimal notation, or with the module 
// name. (The IPvXAddressResolver class is used to resolve the address.)
// INET also defines several NED functions that can be useful: 
//    - moduleListByPath(): TODO: leirni mit csinal, pelda
//    - moduleListByNedType(): TODO: leirni mit csinal, pelda
//
// The peer can be UDPSink or another UDPBasicBurst.
//
// <b>Bursts:</b>
//
// The first burst starts at startTime. Bursts start by immediately sending 
// a packet; subsequent packets are sent at messageFreq intervals. The 
// messageFreq parameter can be a random value, e.g. exponential(10ms).
// A constant interval with jitter can be specified as 1s+uniform(-0.01s,0.01s)
// or uniform(0.99s,1.01s). The length of the burst is controlled by the
// burstDuration parameter. (Note that if messageFreq is greater than 
// burstDuration, the burst will consist of one packet only.) The time between
// burst is the sleepDuration parameter; this can be zero (zero is not
// allowed for burstDuration and messageFreq.)
//
// TODO: ellenorizni a kodban, hogy tenyleg igy legyen: nulla burstDuration es
// messageFreq azonnal generaljon hibat, igy garantalhato hogy haladjon az ido.
// sleep az lehet 0 is.
//
// <b>Packets</b>
//
// Packet length is controlled by the messageLength parameter.
//
// TODO: dokumentalni, hogy msgId es sourceId-t rak a packetekre, es melyik micsoda!!! 
//
// <b>Operation as sink; statistics</b>
//
// Statistics are collected on incoming packets (i.e. only when the app is 
// a sink.)  
// TODO: is that true?
//
// TODO: delayLimit parameter: mikor el, mit csinal! miert nincs defaultja? (0s, azaz no limit)
// TODO: mit csinal sink-kent?? lasd pl: std::map<int,int> SourceSequence member
//
simple UDPBasicBurst like IUDPApp
{
    parameters:
        string destAddresses; // space-separated list of destination IP addresses, can be empty; see module description for possibilities
        string chooseDestAddrMode @enum("once","perBurst","perSend"); // when to change the destination address
        int destAddrRNG = default(0); // random generator ID for selecting a destination address
        int localPort; // local UDP port number
        int destPort; // remote UDP port number
        double startTime @unit("s") = default(0s); // application start time (start of the first burst)
        double stopTime @unit("s") = default(-1s); // application stop time (no packets are sent after this time); a negative or zero value means no limit
        volatile int messageLength @unit("B"); // length of messages to generate, in bytes
        volatile double burstDuration @unit("s"); // burst duration time (zero not allowed)
        volatile double sleepDuration @unit("s"); // time between bursts (zero allowed)
        volatile double messageFreq @unit("s"); // time between messages during bursts; usually a random value, e.g. 0.1s+uniform(-0.001s,0.001s); zero not allowed
        double delayLimit @unit("s"); // maximum accepted delay for a packet; packets with a bigger delay are discarded
        @statistic[sentPk](title="packets sent"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[recvPk](title="packets received"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[duplPk](title="packets duplicated"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay";record=histogram,stats,vector);

    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

