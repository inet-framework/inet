//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
// Handles \IP multicasts.
//
// Receive datagram with multicast address from IPRouting:
// - duplicate datagram if it is sent to more than one output port
//   (to map multicast address on output port, uses multicast routing table)
// - send copy to local deliver, if multicast address contains address of
//   one of our network interfaces
// - if entry in multicast routing table requires tunneling, send to
//   IPTunneling module, otherwise send to IPFragmentation module
//
// @author: Jochen Reber
// @date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
//
simple IPMulticast
{
    parameters:
        @display("i=block/broadcast_s");
    gates:
        input routingIn;
        output localOut;
        output fragmentationOut;
        output tunnelOut;
}

