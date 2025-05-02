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
    QUIC_C_CLOSE = 3;
    QUIC_C_SEND = 4;
    QUIC_C_RECEIVE = 5;
    QUIC_C_ACCEPT = 8;
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
// Tag that allows to specify the stream.
//
class QuicStreamReq extends TagBase
{
    uint64_t streamID;
}

//
// Base class for QUIC Commands. The most convenient way to handle
// ~QUIC is the QuicSocket class, which hides commands from you.
//
// @see ~QuicCommandCode
//
class QuicCommand extends cObject
{
}

//
// Control info for binding a QUIC socket. To create and bind a socket,
// send a message to the ~Quic module with kind=QUIC_C_CREATE_PCB and a
// ~QuicBindCommand attached.
//
// @see ~QuicCommandCode
//
class QuicBindCommand extends QuicCommand
{
    L3Address localAddr;
    uint16_t localPort;
}

//
// Control info to be used for active QUIC open (connect).
//
// @see ~QuicCommandCode
//
class QuicOpenCommand extends QuicCommand
{
    L3Address remoteAddr;
    uint16_t remotePort;
}

//
// App command to let the QUIC know about expected data size to read.
//
// @see ~QuicCommandCode
//
class QuicRecvCommand extends QuicCommand
{
    uint64_t streamID;
    int64_t expectedDataSize;
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
