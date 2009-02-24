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


//
// Pre-routing handling of \IP datagrams: check for errors and decrement TTL.
//
// Handling:
//   - receive IPDatagram
//   - check for header error -> throw away and notify ICMP module on error
//   - decrement hop counter
//   - send correct datagram to IPRouting module
//
// Author: Jochen Reber, date: 2.5.00, 9.5.00, 20.5.00, 9.6.00
// Redesign: Andras Varga, 02.2004
//
simple IPPreRouting
{
    parameters:
        @display("i=block/routing_s");
    gates:
        input in[];
        output routingOut;
}

