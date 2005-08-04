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
simple TCPGenericSrvApp
    parameters:
        address: string, // local address; may be left empty ("")
        port: numeric const, // port number to listen on
        replyDelay: numeric const;
    gates:
        in: tcpIn;
        out: tcpOut;
endsimple

