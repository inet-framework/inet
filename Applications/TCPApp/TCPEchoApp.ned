//
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
// Accepts any number of incoming TCP connections, and sends back whatever
// message arrives on them. The reply can be delayed by a constant time.
// The amount of data sent back is constant times the amount of received data.
// If echoFactor=1.3, then 30% more bytes will be sent back.
//
simple TCPEchoApp
    parameters:
        address: string, // local address; may be left empty ("")
        port: numeric const, // port number to listen on
        echoFactor: numeric const,
        echoDelay: numeric const;
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

