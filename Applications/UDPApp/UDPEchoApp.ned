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


//
// Similar to UDPBasicApp, but it sends back the packet after reception.
//
simple UDPEchoApp
    parameters:
        local_port : numeric const,
        dest_port : numeric const,
        message_length : numeric const, // (bits)
        message_freq : numeric, // should usually be a random value, e.g. exponential(1)
        dest_addresses: string; // list of \IP addresses, separated by spaces
    gates:
        in: from_udp;
        out: to_udp;
endsimple



