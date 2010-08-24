//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010 Zoltan Bojthe
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


// TCPCommmand kommunikáció:
// 
// SEND:
//    Sikeres küldés esetén (ACKed) felküldi a TCP az elküldött byte számot és a send queue méretét.
//
// Receive: OPEN-nél paraméter, hogy régi módon dolgozzon, vagy csak értesítsen az adatról...
//   Értesítés módban:
//     ha volt READ(n), akkor a TCP felküldi az adatból ami belefér (<=n).
//     ha nem volt READ vagy maradt még adat, akkor küld egy TCP_I_DATA_ARRIVED msg-t.
//     A receive queue-ra meg lehet adni az OPEN-nél egy limitet, aminél több nem lehet a bufferben, ilyenkor a tcp úgy állítja az ablakméretet, hogy csak ennyi adatot fogadhasson.
//   Eredeti módban:
//     nincs értelme a limitnek, mert úgyis felküldi, amint megérkezett.
//
// Problémák:
//   MsgBased kommunikáció esetén:
//     Kell valami msg slice-t küldeni értesítési módban, hogy ürüljön a buffer. (Jelenleg csak akkor küldi fel az msg-t, ha megjött az egész msg.)
//       Ilyenkor a hossz-ellenőrzéseknél figyelni a felküldött sliceok méretére is...
//     Elején, vagy végén menjen-e az msg, vagy választható legyen:
//       Az msg-t a hálón az első bájttal küldeni. A fogadó oldalon értesítési mód esetén az OPEN-ben állítható, hogy első vagy utolsó bájttal adja fel az msg-t.
//
// új OPEN paraméterek:
//   - bool Értesítési mód(ÉM=readNotificationsEnabled) / kompatibilis mód
//   - long ÉM esetén: receive queue size limit
//   - bool ÉM & MsgBased esetén: MsgInFirstPacket/MsgInLastPacket
//   - bool TCP_I_DATA_SENT enabled: Y/N


cplusplus {{
#include "IPvXAddress.h"

#include "ByteArrayMessage.h"

typedef cPacket* cPacketPtr;
}}

class noncobject cPacketPtr;
//struct cPacketPtr;

class noncobject IPvXAddress;

class ByteArrayMessage;

//
// \TCP command codes, sent by the application to TCP. These constants
// should be set as message kind on a message sent to the TCP entity.
//
// @see TCPCommand, TCPOpenCommand, TCP
//
enum TcpCommandCode
{

    TCP_C_OPEN_ACTIVE = 1;   // active open (must carry TCPOpenCommand)
    TCP_C_OPEN_PASSIVE = 2;  // passive open (must carry TCPOpenCommand)
    TCP_C_SEND = 3;          // send data (set on data packet)
    TCP_C_READ = 4;	         // read data
    TCP_C_CLOSE = 5;         // "I have no more data to send"
    TCP_C_ABORT = 6;         // abort connection
    TCP_C_STATUS = 7;        // request status info (TCP_I_STATUS) from TCP
}


//
// \TCP indications, sent by TCP to the application. TCP will set these
// constants as message kind on messages it sends to the application.
//
// @see TCPCommand, TCPStatusInfo, TCP
//
enum TcpStatusInd
{

    TCP_I_DATA = 1;              // data packet (set on data packet)
    TCP_I_URGENT_DATA = 2;       // urgent data (set on data packet)
    TCP_I_ESTABLISHED = 3;       // connection established
    TCP_I_PEER_CLOSED = 4;       // FIN received from remote TCP
    TCP_I_CLOSED = 5;            // connection closed normally (via FIN exchange)
    TCP_I_CONNECTION_REFUSED = 6; // connection refused
    TCP_I_CONNECTION_RESET = 7;  // connection reset
    TCP_I_TIMED_OUT = 8;         // conn-estab timer went off, or max retransm. count reached
    TCP_I_STATUS = 9;            // status info (will carry TCPStatusInfo)
    TCP_I_DATA_ARRIVED = 10;     // number of bytes in receive queue
    TCP_I_DATA_SENT = 11;        // number of sent bytes and number of bytes in send queue
}


//
// Currently not in use.
//
enum TCPErrorCode
{

}


//
// Control info for \TCP connections. This class is to be set as control info
// (see cMessage::setControlInfo()) on all messages exchanged between TCP and
// application, in both directions. Some commands and indications
// (TCP_C_OPEN_xxx, TCP_I_STATUS) use subclasses.
//
// connId identifies the connection locally within the application (internally,
// TCP uses the (app gate index, connId) pair to identify the socket).
// connId is to be chosen by the application in the open command.
//
//# TODO explain userId
//
// @see TcpCommandCode, TcpStatusInd, TCPOpenCommand, TCPStatusInfo, TCP
//
class TCPCommand
{
    int connId = -1;   // identifies the socket within the application
    int userId = -1;   // id than can be freely used by the app
}


//
// Currently not in use.
//
class TCPErrorInfo extends TCPCommand
{
    int errorCode enum(TCPErrorCode);
    string messageText;
}

//
// Defines what to transmit as payload in TCP segments
//
enum TCPDataTransferMode
{
    TCP_TRANSFER_UNDEFINED = 0;         // Invalid value
    TCP_TRANSFER_BYTECOUNT = 1;         // Transmit byte counts only
    TCP_TRANSFER_OBJECT = 2;            // Transmit the application packet C++ objects
    TCP_TRANSFER_BYTESTREAM = 3;        // Transmit actual bytes
}

//
// Control info to be used for active or passive TCP open.
//
// localAddr, remoteAddr, localPort, remotePort should be self-explanatory.
// localAddr is optional because TCP can learn it from IP when a packet
// is received from the peer; localPort is optional because TCP supports
// ephemeral ports.
//
// The sendQueueClass, receiveQueueClass and tcpAlgorithmClass fields
// allow per-connection TCP configuration. These fields may contain
// names of classes subclassed from TCPSendQueue, TCPReceiveQueue
// and TCPAlgorithm, respectively. If not set, module parameters with
// similar names are used.
//
// The fork parameter is used with passive open, and controls what happens
// when an incoming connection is received. With fork=true, it emulates
// the Unix accept(2) syscall semantics: a new connection structure
// is created for the connection (with a new connId, see in TCPCommand),
// and the connection structure with the old connId remains listening.
// With fork=false, all the above does not happen: the first connection
// is accepted (with the original connId), and further incoming connections
// will be refused by TCP by sending an RST segment.
//
// @see TcpCommandCode, TCP
//
class TCPOpenCommand extends TCPCommand
{
    IPvXAddress localAddr;    // may be left empty
    IPvXAddress remoteAddr;   // required for active open
    int localPort = -1;       // required for passive open
    int remotePort = -1;      // required for active open
    int dataTransferMode enum(TCPDataTransferMode); // whether to transmit C++ objects, real bytes or just byte counts
    bool fork = false;        // used only for passive open
    bool explicitReadsEnabled;		// when enabled: TCP send up only TCPDataArrivedInfo notification when received some data, and send the packet only after a READ msg.
    bool sendNotificationsEnabled;  // when enabled: when TCP sent some data to partner, it's send up a TCPDataSentInfo notification.
    bool sendingObjectUpAtFirstByteEnabled; // when enabled: when TCPDataTransferMode is "object", then TCP send up object at first byte of msg
    long receiveBufferSize;         // max size of used receive queue in TCP layer (SO_RCVBUF). only valid when explicitReadsEnabled is true 
    string tcpAlgorithmClass;       // TCP congestion control algorithm; leave empty for default
}


//
// Control info to be used with the SEND command.
//
// @see TcpCommandCode, TCP
//
class TCPSendCommand extends TCPCommand
{
}


//
// Control info to be used with the READ command.
//
// @see TcpCommandCode, TCP
//
class TCPReadCommand extends TCPCommand
{
    long bytes;
}


//
// Sent with message kind TCP_I_ESTABLISHED, to let the app know
// about the local and remote IP address and port.
//
// @see TcpCommandCode, TCP
//
class TCPConnectInfo extends TCPCommand
{
    IPvXAddress localAddr;
    IPvXAddress remoteAddr;
    int localPort;
    int remotePort;
}


//
// Sent with message kind TCP_I_DATA or TCP_I_URGENT_DATA
//
class TCPDataMsg extends ByteArrayMessage
{
    @customize(true);
    cPacketPtr dataObject;
    bool isBegin;	// true when first byte of dataObject is first byte of the packet, false when last byte of dataObject is last byte of the packet   
}


//
// Sent with message kind TCP_I_STATUS, in response to command TCP_C_STATUS.
// For explanation of variables, see RFC 793 or TCPStateVariables in
// TCPConnection.h.
//
// @see TcpStatusInd, TcpCommandCode, TCP
//
class TCPStatusInfo extends TCPCommand
{
    int state;
    string stateName;

    IPvXAddress localAddr;
    IPvXAddress remoteAddr;
    int localPort;
    int remotePort;

    unsigned int snd_mss;

    unsigned int snd_una;
    unsigned int snd_nxt;
    unsigned int snd_max;
    unsigned int snd_wnd;
    unsigned int snd_up;
    unsigned int snd_wl1;
    unsigned int snd_wl2;
    unsigned int iss;

    unsigned int rcv_nxt;
    unsigned int rcv_wnd;
    unsigned int rcv_up;
    unsigned int irs;

    bool fin_ack_rcvd;
}

//
// TODO leiras!!!
//
// @see TcpStatusInd, TcpCommandCode, TCP
//
class TCPDataArrivedInfo extends TCPCommand
{
    long availableBytesInReceiveQueue;
}

//
// TODO leiras!!!
//
// @see TcpStatusInd, TcpCommandCode, TCP
//
class TCPDataSentInfo extends TCPCommand
{
    long availableBytesInSendQueue;
    unsigned int sentBytes;
}
