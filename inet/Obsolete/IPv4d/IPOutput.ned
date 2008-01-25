//
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
// Receive complete datagram from IPFragmentation:
// - throw away if ttl==0
// - otherwise send to the network interface on gate queueOut
//
// Author: Jochen Reber, 2000
// Redesign: Andras Varga, 02.2004
//
simple IPOutput
{
    parameters:
        @display("i=block/queue");
    gates:
        input in;
        input arpIn;
        output queueOut;
}

