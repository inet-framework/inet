//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import inet.networklayer.common.L3Address;
import inet.common.TagBase;

namespace inet;


//
// QUIC command codes, sent by the application to ~QUIC. These constants
// should be set as message kind on messages sent to the ~QUIC entity.
//
// @see ~QUICControlInfo, ~QUICStatusInd, ~QUIC
//
enum QuicCommandCode
{
    QUIC_C_CREATE_PCB = 0;
    QUIC_C_OPEN_PASSIVE = 1;
    QUIC_C_OPEN_ACTIVE = 2;
    QUIC_C_SEND_APP_CLOSE = 3;
    QUIC_C_SEND = 4;
    QUIC_C_RECEIVE = 5; //app notify QUIC about amount of data ready to read
    QUIC_C_SET_STREAM_PRIO = 6;
    QUIC_C_QUEUE_MSGS_LIMIT = 7; //for trafficgen
    QUIC_C_ACCEPT = 8;        // accept connection
}


//
// QUIC indications, sent by ~QUIC to the application. ~QUIC will set these
// constants as message kind on messages it sends to the application.
//
// @see ~QUICControlInfo, ~QUICCommandCode, ~QUIC
//
enum QuicStatusInd
{
    QUIC_I_DATA = 0;  // Attached to requested data packet
    QUIC_I_DATA_NOTIFICATION = 1; // Notifiy application when data has arrived
    QUIC_I_ERROR = 2; // ~QUIC attaches ~QUICIndication to received data packet
    QUIC_I_ESTABLISHED = 3;
    QUIC_I_CLOSED = 4;
    QUIC_I_SEND_MSG = 5; //for trafficgen
    QUIC_I_QUEUE_LIMIT = 6; //for trafficgen
    QUIC_I_AVAILABLE = 7;         // conncetion available
    QUIC_I_SENDQUEUE_FULL = 8;
    QUIC_I_SENDQUEUE_DRAIN = 9;
    QUIC_I_MSG_REJECTED = 10;
}


//
// Base class for QUIC Command class. The most convenient way to handle
// ~QUIC is the QUICSocket class, which hides commands from you.
//
// sockId must uniquely identify the socket in the ~QUIC module; the best way
// to achieve that is to obtain the value from QUICSocket::generateSocketId().
//
// @see ~QUICCommandCode
//
class QuicCommand extends cObject
{
    //uint64_t connId = -1;
}


//
// Control info to be used for active or passive QUIC open.
//
// localAddr, remoteAddr, localPort, remotePort should be self-explanatory.
//
// @see ~QUICCommandCode, ~IQUIC
//
class QuicOpenCommand extends QuicCommand
{
    L3Address localAddr; // may be left empty
    L3Address remoteAddr;// required for active open
    int localPort = -1;       // required for passive open
    int remotePort = -1;      // required for active open
    uint64_t maxNumStreams;
}

//
// Control info for binding an QUIC socket. To create and bind a socket,
// send a message to the ~QUIC module with kind=QUIC_C_CREATE_PCB and an
// ~QUICBindCommand attached.
//
// Both the address and the port may be left unset. If the port is
// unset, ~QUIC will assign an ephemeral port.
//
// @see ~QUICCommandCode
//
class QuicBindCommand extends QuicCommand
{
    L3Address localAddr;
    int localPort = -1;
}

//
// Control info for sending data via QUIC. To send a packet, send it to
// the ~QUIC module with kind=QUIC_C_SEND and an ~QUICSendCommand attached.
//
// QUIC_C_SEND/~QUICSendCommand models both the Unix send() and sendto() calls.
// If the socket is connected, destAddr/destPort may be left blank (send()),
// otherwise it must contain the destination for the packet (sendto()).
//
// @see ~QUICCommandCode
//
class QuicSendCommand extends QuicCommand
{
    L3Address destAddr; // destination IPv4 or IPv6 address
    int destPort = -1;     // destination port
    L3Address srcAddr;       // source address
    int interfaceId = -1; // interface on which the packet must be sent (see ~InterfaceTable)
    long streamID = -1;
    uint32_t streamPriority = 0;
}

class QuicStreamReq extends TagBase
{
    long streamID = -1;
}

//
// APP control info to let the QUIC know about expected data size to read
//
class QuicRecvCommand extends QuicCommand
{
    long streamID = -1;
    long expectedDataSize = -1; 
}

//
//QUIC control info to let the app know about available data size to read.
//Sent with message kind QUIC_I_DATA_NOTIFICATION
//
class QuicDataAvailableInfo extends QuicCommand
{
    long streamID = -1;
    long avaliableDataSize = -1;
}

//
// Control info to be used to accept an available connection.
//
class QuicAcceptCommand extends QuicCommand
{
}

//
// Sent with message kind QUIC_I_AVAILABLE, to let the app know
// about the local and remote IP address and port.
//
// @see ~QuicCommandCode, ~IQuic
//
class QuicAvailableInfo extends QuicCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
    int newSocketId;
}

//
// Sent with message kind QUIC_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see ~QuicCommandCode, ~IQuic
//
class QuicConnectInfo extends QuicCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    int localPort;
    int remotePort;
}

//
// Control info for closing an QUIC socket. To close a socket, send a message
// to the ~QUIC module with kind=QUIC_C_CLOSE and an ~QUICCloseCommand attached.
//
// @see ~QUICCommandCode
//
class QuicCloseCommand extends QuicCommand
{
}

//packet QuicSimpleMessage
//{
//    uint32 dataLen;
//    char data[];
//    simtime_t creationTime = 0;
//    bool encaps = false;
//}

