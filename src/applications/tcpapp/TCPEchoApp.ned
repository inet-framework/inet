//
// Copyright (C) 2004 Andras Varga
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


package inet.applications.tcpapp;

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
simple TCPEchoApp like TCPApp
{
    parameters:
        string address = default(""); // local address; may be left empty ("")
        int port; // port number to listen on
        double echoFactor = default(1);
        double echoDelay @unit("s") = default(0s);
        @display("i=block/app");
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

