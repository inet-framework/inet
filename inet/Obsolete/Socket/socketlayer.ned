//
//-----------------------------------------------------------------------------
//-- fileName: socketlayer.ned
//--
//-- Module declaration for the Socket Layer
//-----------------------------------------------------------------------------
//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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

simple SocketLayer
{
    parameters:
        bool debug;
        @display("i=block/wheelbarrow");
    gates:
        input from_ip;
        input from_tcp;
        input from_udp;
        input from_appl[];

        output to_ip;
        output to_tcp;
        output to_udp;
        output to_appl[];
}
