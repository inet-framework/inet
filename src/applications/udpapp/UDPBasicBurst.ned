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
// TODO: forditani:
// A sajat magara mutato cimet kihagyja a felsoroltak kozul, mintha bele se irtak volna.
// 
// An address may be given in the dotted decimal notation, or with the module 
// name. (The IPvXAddressResolver class is used to resolve the address.)
// TODO: forditani:
// Hasznalhato a "Broadcast" string is, ez esetben broadcast uzeneteket kuld.
//
// INET also defines several NED functions that can be useful: 
//    - moduleListByPath(): TODO: forditani:
//          visszaadja egy stringben szokozokkel elvalasztva a a parameterekre match-elo modulok neveit.
//          See cTopology::extractByModulePath() function
//          example: destaddresses = moduleListByPath("**.host[*]", "**.fixhost[*]")
//    - moduleListByNedType(): TODO: forditani: 
//          visszaadja egy stringben szokozokkel elvalasztva a modulok neveit.
//          The type name is fully qualified.
//          See cTopology::extractByNedTypeName() function
//          example: destaddresses = moduleListByNedType("inet.nodes.inet.StandardHost")
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
// allowed for messageFreq.) The zero burstDuration is implemented as infinity.
//
// The zero value in burstDuration or messageFreq parameters generates a runtime error.
// The sleepDuration parameter accepts zero value.
//
// <b>Packets</b>
//
// Packet length is controlled by the messageLength parameter.
//
// TODO: forditani
// A modul kuldeskor sourceID = source modul ID, es msgId parametereket rak a packetekre,
// fogadaskor ezek alapjan ellenorzi, hogy helyes sorrendben erkeznek-e a csomagok.
// Az msgId minden kuldeskor novekszik 1-gyel, fuggetlenul a destaddress-tol.
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
// TODO: forditani
//  - outOfOrderPk: a nem megfelelo sorrendben erkezett csomagok statisztikaja
//        (a csomag akkor jott rossz sorrendben, ha van msgId es sourceId parameter rakaksztva,
//        es mar erkezett azonos sourceID-rol nagyobb vagy azonos msgId-vel uzenet)
//  - dropPk: a jo sorrendben/ellenorizhetetlen sorrendben erkezett csomagok kozul a delayLimit 
//        parameternel nagyobb delay-jel erkezett csomagok statisztikaja
//        delayLimit = 0 eseten nincs limit
//  - rcvdPk: a jo sorrendben/ellenorizhetetlen sorrendben, megfelelo delay-jel erkezett csomagok
//        statisztikaja
//  - endToEndDelay: a jo sorrendben/ellenorizhetetlen sorrendben, megfelelo delay-jel erkezett
//        csomagok end to end delay statisztikaja
//
simple UDPBasicBurst like IUDPApp
{
    parameters:
        string destAddresses; // space-separated list of destination IP addresses, can be empty;
                              // see module description for possibilities
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
        double delayLimit @unit("s") = default(0); // maximum accepted delay for a packet; packets with a bigger delay are discarded (dropped), zero value means no limit
        @statistic[sentPk](title="packets sent"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[rcvdPk](title="packets received"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[outOfOrderPk](title="packets received out of order"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay";record=histogram,stats,vector);

    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

