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
// Accepts any number of incoming TCP connections, and sends back the
// messages that arrive on them, The lengths of the messages are
// multiplied by echoFactor before sending them back (echoFactor=1 will
// result in sending back the same message unmodified.) The reply can also be
// delayed by a constant time (echoDelay parameter).
//
// When TCPEchoApp receives data packets from TCP (and such, when they can be
// echoed) depends on the sendQueue/receiveQueue setting of TCP.
// With TCPVirtualBytesSendQueue/RcvQueue, TCP passes up data to us
// as soon as a segment arrives, so it can be echoed immediately.
// With TCPMsgBasedSendQueue/RcvQueue, our local TCP reproduces the same
// messages that the sender app passed down to its TCP -- so if the sender
// app sent a single 100 MB message, it will be echoed only when all
// 100 megabytes have arrived.
//
// Compatible with both IPv4 and IPv6.
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

