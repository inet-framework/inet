//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    QUIC_C_CREATE_PCB = 1;
    QUIC_C_OPEN_PASSIVE = 2;
    QUIC_C_OPEN_ACTIVE = 3;
    QUIC_C_CLOSE = 4;
    QUIC_C_SEND = 5;
    QUIC_C_RECEIVE = 6;
    QUIC_C_ACCEPT = 7;
}


//
// QUIC indications, sent by ~QUIC to the application. ~QUIC will set these
// constants as message kind on messages it sends to the application.
//
// @see ~QUICControlInfo, ~QUICCommandCode, ~Quic
//
enum QuicStatusInd
{
    QUIC_I_DATA = 1;
    QUIC_I_DATA_AVAILABLE = 2;
    QUIC_I_ERROR = 3;
    QUIC_I_ESTABLISHED = 4;
    QUIC_I_CLOSED = 5;
    QUIC_I_CONNECTION_AVAILABLE = 6;
    QUIC_I_SENDQUEUE_FULL = 7;
    QUIC_I_SENDQUEUE_DRAIN = 8;
    QUIC_I_MSG_REJECTED = 9;
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
// ~Quic is the QuicSocket class, which hides commands from you.
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
// App command to let QUIC know about expected data size to read.
//
// @see ~QuicCommandCode
//
class QuicRecvCommand extends QuicCommand
{
    uint64_t streamID;
    int64_t expectedDataSize;
}

//
// Control info to be used to accept an available connection.
//
class QuicAcceptCommand extends QuicCommand
{
    int newSocketId;
}

//
// Sent with message kind QUIC_I_CONNECTION_AVAILABLE, to let the app know
// about the local and remote IP address and port.
//
// @see ~QuicCommandCode, ~IQuic
//
class QuicConnectionAvailableInfo extends QuicCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    uint16_t localPort;
    uint16_t remotePort;
    int newSocketId;
}

//
// QUIC control info to let the app know about available data size to read.
// Sent with message kind QUIC_I_DATA_AVAILABLE.
//
class QuicDataInfo extends QuicCommand
{
    uint64_t streamID;
    uint64_t avaliableDataSize;
}

//
// Sent with message kind QUIC_I_ESTABLISHED to let the app know
// about the local and remote IP address and port.
//
// @see ~QuicCommandCode, ~IQuic
//
class QuicConnectionInfo extends QuicCommand
{
    L3Address localAddr;
    L3Address remoteAddr;
    uint16_t localPort;
    uint16_t remotePort;
}
