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
// The own addresses will be ignored.
//
// An address may be given in the dotted decimal notation, or with the module
// name. (The IPvXAddressResolver class is used to resolve the address.)
// You can use the "Broadcast" string as address for sending broadcast messages.
//
// INET also defines several NED functions that can be useful:
//    - moduleListByPath("pattern",...):
//          Returns a space-separated list of the modulenames.
//          All modules whole getFullPath() matches one of the pattern parameters will get included.
//          The patterns may contain wilcards in the same syntax as in ini files.
//          See cTopology::extractByModulePath() function
//          example: destaddresses = moduleListByPath("**.host[*]", "**.fixhost[*]")
//    - moduleListByNedType("fully.qualified.ned.type",...):
//          Returns a space-separated list of the modulenames with the given NED type(s).
//          All modules whose getNedTypeName() is listed in the given parameters will get included.
//          The NED type name is fully qualified.
//          See cTopology::extractByNedTypeName() function
//          example: destaddresses = moduleListByNedType("inet.nodes.inet.StandardHost")
//
// The peer can be UDPSink or another UDPBasicBurst.
//
// <b>Bursts:</b>
//
// The first burst starts at startTime. Bursts start by immediately sending
// a packet; subsequent packets are sent at sendInterval intervals. The
// sendInterval parameter can be a random value, e.g. exponential(10ms).
// A constant interval with jitter can be specified as 1s+uniform(-0.01s,0.01s)
// or uniform(0.99s,1.01s). The length of the burst is controlled by the
// burstDuration parameter. (Note that if sendInterval is greater than
// burstDuration, the burst will consist of one packet only.) The time between
// burst is the sleepDuration parameter; this can be zero (zero is not
// allowed for sendInterval.) The zero burstDuration is implemented as infinity.
//
// The zero value in burstDuration or sendInterval parameters generates a runtime error.
// The sleepDuration parameter accepts zero value.
//
// <b>Packets</b>
//
// Packet length is controlled by the messageLength parameter.
//
// The module adds two parameters to packets before sending:
//  - sourceID: source module ID
//  - msgId: incremented by 1 after send any packet.
// When received packet has this parameters, the module checks the order of received packets.
//
// <b>Operation as sink</b>
//
// When destAddresses parameter is empty, the module receives packets and makes statistics only.
//
// <b>Statistics</b>
//
// Statistics are collected on outgoing packets:
//  - sentPk: packet object
//
// Statistics are collected on incoming packets:
//  - outOfOrderPk: statistics of out of order packets.
//        The packet is out of order, when has msgId and sourceId parameters and module
//        received bigger msgId from same sourceID.
//  - dropPk: statistics of dropped packets.
//        The packet is dropped when not out-of-order packet and delay time is larger than
//        delayLimit parameter. The delayLimit=0 is infinity.
//  - rcvdPk: statistics of not dropped, not out-of-order packets.
//  - endToEndDelay: end to end delay statistics of not dropped, not out-of-order packets.
//
simple UDPBasicBurst like IUDPApp
{
    parameters:
        string destAddresses; // space-separated list of destination IP addresses, can be empty;
                              // see module description for possibilities
        string chooseDestAddrMode @enum("once","perBurst","perSend"); // when to change the destination address
        int destAddrRNG = default(0); // random generator ID for selecting a destination address
        int localPort = default(-1);  // local UDP port number (-1: use ephemeral port)
        int destPort; // remote UDP port number
        double startTime @unit(s) = default(1s); // application start time (start of the first burst)
        double stopTime @unit(s) = default(-1s); // application stop time (no packets are sent after this time); a negative or zero value means no limit
        volatile int messageLength @unit(B); // length of messages to generate, in bytes
        volatile double burstDuration @unit(s); // burst duration time (zero not allowed)
        volatile double sleepDuration @unit(s); // time between bursts (zero allowed)
        volatile double sendInterval @unit(s); // time between messages during bursts; usually a random value, e.g. 0.1s+uniform(-0.001s,0.001s); zero not allowed
        double delayLimit @unit(s) = default(0); // maximum accepted delay for a packet; packets with a bigger delay are discarded (dropped), zero value means no limit
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[dropPk](type=cPacket);
        @signal[outOfOrderPk](type=cPacket);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; source=dropPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[outOfOrderPk](title="packets received out of order"; source=outOfOrderPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

