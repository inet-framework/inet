//
// Copyright (C) 2005 Andras Babos
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


package org.omnetpp.inet.Applications.UDPApp;

//
// Similar to UDPBasicApp, but it sends back the packet after reception.
//
simple UDPEchoApp like UDPApp
{
    parameters:
        int local_port;
        int dest_port;
        int message_length @unit("b"); // (bits)
        volatile double message_freq @unit("s"); // should usually be a random value, e.g. exponential(1)
        string dest_addresses = default(""); // list of \IP addresses, separated by spaces
    gates:
        input from_udp;
        output to_udp;
}



