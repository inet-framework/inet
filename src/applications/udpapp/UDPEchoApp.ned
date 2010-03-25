//
// Copyright (C) 2005 Andras Babos
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
// Similar to UDPBasicApp, but it sends back the packet after reception.
//
simple UDPEchoApp like UDPApp
{
    parameters:
        int localPort;
        int destPort;
        volatile int messageLength @unit("B"); // length of messages to generate, int bytes
        volatile double messageFreq @unit("s"); // should usually be a random value, e.g. exponential(1)
        string destAddresses = default(""); // list of \IP addresses, separated by spaces
        @display("i=block/app");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}



