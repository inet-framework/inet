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
// TCP protocol implementation. Supports RFC 793, RFC 1122, RFC 2001.
//
// A TCP segment is represented by the class TCPSegment.
//
// For communication between client applications and TCP, the TcpCommandCode
// and TcpStatusInd enums are used as message kinds, and TCPCommand
// and its subclasses are used as control info.
//
// To open a connection from a client app, send a cMessage to TCP with
// TCP_C_OPEN_ACTIVE as message kind and a TCPOpenCommand object filled in
// and attached to it as control info. (The peer TCP will have to be LISTENing;
// the server app can achieve this with a similar cMessage but TCP_C_OPEN_PASSIVE
// message kind.)
//
// The client app can send data by assigning the TCP_C_SEND message kind
// and attaching a TCPSendCommand control info object to the data packet,
// and sending it to TCP. The server app will receive data as messages
// with the TCP_I_DATA message kind and TCPSendCommand control info.
// (Whether you'll receive the same or identical messages, or even whether
// you'll receive data in the same sized chunks as sent depends on the
// sendQueueClass and receiveQueueClass used, see below. With
// TCPVirtualDataSendQueue and TCPVirtualDataRcvQueue set, message objects
// and even message boundaries are not preserved.)
//
// To close, the client sends a cMessage to TCP with the TCP_C_CLOSE message kind
// and TCPCommand control info.
//
// TCP sends notifications to the application whenever there's a significant
// change in the state of the connection: established, remote TCP closed,
// closed, timed out, connection refused, connection reset, etc. These
// notifications are also cMessages with message kind TCP_I_xxx
// (TCP_I_ESTABLISHED, etc.) and TCPCommand as control info.
//
// One TCP module can serve several application modules, and several
// connections per application. The <i>k</i>th application connects to TCP's
// from_appl[k] and to_appl[k] ports. When talking to applications, a
// connection is identified by the (application port index, connId) pair,
// where connId is assigned by the application in the OPEN call.
//
// The TCPSocket C++ class is provided to simplify managing TCP connections
// from applications. TCPSocket handles the job of assembling and sending
// command messages (OPEN, CLOSE, etc) to TCP, and it also simplifies
// the task of dealing with packets and notification messages coming from TCP.
//
// The module parameters sendQueueClass and receiveQueueClass should be
// set the names of classes that manage the actual send and receive queues.
// Currently they have to be set to "TCPVirtualDataSendQueue" and
// "TCPVirtualDataRcvQueue" -- these classes manage "virtual bytes",
// that is, only byte counts are transmitted over the TCP connection
// and no actual data. (cMessage contents, and even message boundaries
// are not preserved with these classes.) In the future, other send queue
// and receive queue classes will be implemented which will allow actual
// raw bytes and (with different queue classes) cMessages to be transmitted.
//
// The TCP flavour supported depends on the value of the tcpAlgorithmClass
// module parameters. Currently it has to be set to "TCPTahoeReno", which
// implements TCP Tahoe or (by enabling a flag) TCP Reno. In the future, other
// classes can be written which implement New Reno, Vegas, LinuxTCP or other
// variants.
//
// The TCP module itself implements the following:
//  - ...
//
// The TCPTahoeReno algorithm implements:
//  - ...
//
// The TCP module doesn't implement:
//  - ...
//
// TCPTahoeReno doesn't implement:
//  - ...
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





