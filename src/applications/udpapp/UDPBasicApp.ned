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


package inet.applications.udpapp;

//
// Sends \UDP packets to the given \IP address at the given interval.
// Compatible with both IPv4 and IPv6.
//
// The interval can be a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly chosen for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The IPAddressResolver
// class is used to resolve the address.) To disable the model, set
// destAddresses to "".
//
// The peer can be UDPSink or another UDPBasicApp (it handles received packets
// exactly like UDPSink).
//
simple UDPBasicApp like UDPApp
{
    parameters:
        int localPort; // if -1, app doesn't bind in UDP
        int destPort;
        volatile int messageLength @unit("B"); // length of messages to generate, in bytes
        volatile double messageFreq @unit("s"); // should usually be a random value, e.g. exponential(1)
        string destAddresses = default(""); // list of \IP addresses, separated by spaces
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}



