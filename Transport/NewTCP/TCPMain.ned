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
// TCP protocol implementation. Supports (will support) RFC 793, RFC 1122, 
// RFC 2001.
//
// The application can send the following commands:
//  - <tt>TCP_C_OPEN_PASSIVE</tt>: server app ready to accept incoming connection
//    (like LISTEN)
//  - <tt>TCP_C_OPEN_ACTIVE</tt>: open a connection (server must have already
//    issued passive open)
//  - <tt>TCP_C_SEND</tt>
//  - <tt>TCP_C_CLOSE</tt>: close connection
//  - <tt>TCP_C_ABORT</tt>
//  - <tt>TCP_C_STATUS</tt>
//
// The following indications are sent from TcpModule to the application:
//  - FIXME write it
//
// The commands and indications are represented by the message kind 
// and control info class TCPCommand and subclasses.
//
simple TCPMain
    parameters:
      sendQueueClass: string,
      receiveQueueClass: string,
      tcpAlgorithmClass: string;
    gates:
      in: from_appl[], from_ip;
      out: to_appl[], to_ip;
endsimple





