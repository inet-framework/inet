//
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


//
// Consumes and prints packets received from the UDP module.
//
simple UDPSink
    gates:
        in: from_udp;
        out: to_udp;
endsimple


//
// Sends UDP packets to the given \IP address at the given interval.
// The interval can be a constant or a random value (e.g. exponential(1)).
// If the dest_addresses parameter contains more than one address, one
// of them is randomly for each packet. An address may be given in the
// dotted decimal notation, or with the module name. (The IPAddressResolver
// class is used to resolve the address.) To disable the model, set
// dest_addresses to "".
//
// The peer can be UDPSink or another UDPApp (it handles received packets
// exactly like UDPSink).
//
//#FIXME change message length to BYTES
//
simple UDPApp
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



