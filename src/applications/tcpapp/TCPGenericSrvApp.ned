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
// Generic server application for modelling TCP-based request-reply style
// protocols or applications.
//
// Requires message object preserving sendQueue/receiveQueue classes
// to be used with TCP (that is, TCPMsgBasedSendQueue and TCPMsgBasedRcvQueue;
// TCPVirtualBytesSendQueue/RcvQueue are not good).
//
// The module accepts any number of incoming \TCP connections, and expects
// to receive messages of class GenericAppMsg on them. A message should
// contain how large the reply should be (number of bytes). TCPGenericSrvApp
// will just change the length of the received message accordingly, and send
// back the same message object. The reply can be delayed by a constant time
// (replyDelay parameter).
//
// Compatible with both IPv4 and IPv6.
//
// @see GenericAppMsg, TCPBasicClientApp, TelnetApp
//
simple TCPGenericSrvApp like TCPApp
{
    parameters:
        string address = default(""); // local address; may be left empty ("")
        int port = default(1000);     // port number to listen on
        double replyDelay @unit("s") = default(0s);
        @display("i=block/app");
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}

