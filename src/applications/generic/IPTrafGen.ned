//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


package inet.applications.generic;

//
// Consumes and prints packets received from the IP module.
// Compatible with both IPv4 and IPv6.
//
simple IPTrafSink like IPTrafficGenerator
{
    @display("i=block/sink");
    gates:
        input ipIn @labels(IPControlInfo/up);
        input ipv6In @labels(IPv6ControlInfo/up);
        output ipOut @labels(IPControlInfo/down);
        output ipv6Out @labels(IPv6ControlInfo/down);
}


//
// Sends IP or IPv6 datagrams to the given address at the given interval.
// The interval can be a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly for each packet. An address may be given in the
// dotted decimal notation (or, for IPv6, in the usual notation with colons),
// or with the module name. (The IPAddressResolver class is used to resolve
// the address.) To disable the model, set destAddresses to "".
//
// The peer can be IPTrafSink or another IPTrafGen (it handles received packets
// exactly like IPTrafSink).
//
simple IPTrafGen like IPTrafficGenerator
{
    parameters:
        double startTime @unit("s") = default(1s); // the time application generates first packet
        volatile double packetInterval @unit("s") = default(10ms); // may be a random value, e.g. exponential(1)
        int numPackets = default(0); // max number of packets to generate
        int protocol; // value for IPDatagram protocol field
        volatile int packetLength @unit("B"); // in bytes (changed 2005-11-04)
        string destAddresses = default(""); // list of \IP addresses, separated by spaces
        @display("i=block/source");
    gates:
        input ipIn @labels(IPControlInfo/up);
        input ipv6In @labels(IPv6ControlInfo/up);
        output ipOut @labels(IPControlInfo/down);
        output ipv6Out @labels(IPv6ControlInfo/down);
}



