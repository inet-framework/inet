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



/**
 * @defgroup TCP  TCP protocol
 *
 * Usage of TCP is described with the TCPMain class.
 *
 * The TCP protocol implementation is composed of several classes. TCPMain
 * is a simple module, it manages socketpair-to-connection mapping, and
 * dispatches segments and user commands to the appropriate TCPConnection
 * object.
 *
 * TCPConnection stores the state vars (TCB), sends/receives SYN, FIN, RST
 * segments and manages all state transitions (LISTEN->SYN_SENT etc).
 * TCPConnection internally relies on 3 objects, subclassed from TCPSendQueue,
 * TCPReceiveQueue and TCPAlgorithm, respectively. The first two manage
 * the actual data (bytes, "virtual" bytes, message objects etc) so
 * TCPConnection only needs to work with sequence number variables.
 * Managing ACKs, retransmissions, etc are "outsourced" from TCPConnection
 * into TCPAlgorithm, so things like slow start, fast rexmit, SACK etc can
 * be conveniently implemented in TCPAlgorithm subclasses, without further
 * littering the basic protocol implementation in TCPConnection.
 *
 * A TCP segment is represented by the class TCPSegment (defined in a .msg file).
 * TCPCommand is the control info base class used for communication between
 * TCP and the application layer; command-specific control info classes
 * (TCPOpenCommand, etc.) are subclasses from TCPCommand. Control info classes
 * are also defined in a .msg file.
 *
 * There is also a TCPSocket utility class which can be used from application
 * models. TCPSocket handles the job of assembling and sending command messages
 * (OPEN, CLOSE, etc) to TCP, and thus it makes it easier for you to open,
 * close, and send data on TCP connections. It also simplifies the task of
 * dealing with packets and notification messages coming from the TCP module.
 *
 * A summary of classes used:
 *    - TCPMain: module class
 *    - TCPConnection: manages a connection
 *    - TCPSendQueue, TCPReceiveQueue: abstract base classes for various types
 *          of send and receive queues.
 *    - TCPAlgorithm: abstract base class for TCP algorithms, from retransmissions
 *          to congestion control and SACK
 *    - TCPSegment: TCP segment
 *    - TCPCommand: control info for communication between TCP and application
 *    - TCPSocket: utility class to simplify talking to TCP from applications
 */
