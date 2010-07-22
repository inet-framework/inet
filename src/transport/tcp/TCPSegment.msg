//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009-2010 Thomas Reschka
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


cplusplus {{
#include <iostream>
#include "INETDefs.h"

    // default TCP header length: 20 bytes
    #define TCP_HEADER_OCTETS  20    // without options

    // maximum TCP header length (base+options): 60 = 4 * 15 bytes
    const unsigned int TCP_MAX_HEADER_OCTETS = 60;

    // maximum allowed sack entry number, if no other options are used
    const unsigned int MAX_SACK_ENTRIES = 4;

    typedef cPacket *cPacketPtr;

    inline std::ostream& operator<<(std::ostream& os, cPacketPtr msg)
    {
        return os << "(" << msg->getClassName() << ") " << msg->getName() << ": " << msg->info();
    }

    inline void doPacking(cCommBuffer *b, cPacketPtr& msg) {msg->parsimPack(b);}
    inline void doUnpacking(cCommBuffer *b, cPacketPtr& msg) {msg->parsimUnpack(b);}
}}


struct cPacketPtr;


struct TCPPayloadMessage
{
    unsigned int endSequenceNo;
    cPacketPtr msg;
}

//
// TCP Option Numbers
// Reference: http://www.iana.org/assignments/tcp-parameters/
// Date: 2010-07-04
//
// Note: Options not yet implemented should stay commented out
//
enum TCPOptionNumbers
{
    TCPOPTION_END_OF_OPTION_LIST = 0;                   // RFC 793, LENGTH: 1 Byte
    TCPOPTION_NO_OPERATION = 1;                         // RFC 793, LENGTH: 1 Byte
    TCPOPTION_MAXIMUM_SEGMENT_SIZE = 2;                 // RFC 793, LENGTH: 4 Bytes
    TCPOPTION_WINDOW_SCALE = 3;                         // RFC 1323, LENGTH: 3 Bytes
    TCPOPTION_SACK_PERMITTED = 4;                       // RFC 2018, LENGTH: 2 Bytes
    TCPOPTION_SACK = 5;                                 // RFC 2018, LENGTH: N (max. N = 4) 8 * n + 2 Bytes  => 32 + 2 + 2 * NOP = 36 Bytes; If TIMESTAMP option is used with SACK: max. n = 3 => 12 Bytes (for Timestamp) + 28 Bytes (for SACK) = 40 Bytes
//    TCPOPTION_ECHO = 6;                              // (obsoleted by option 8) RFC 1072, LENGTH: 6 Bytes
//    TCPOPTION_ECHO_REPLY = 7;                        // (obsoleted by option 8) RFC 1072, LENGTH: 6 Bytes
    TCPOPTION_TIMESTAMP = 8;                            // RFC 1323, LENGTH: 10 Bytes
//  TCPOPTION_PARTIAL_ORDER_CONNECTION_PERMITTED = 9;   // RFC 1693, LENGTH: 2 Bytes
//  TCPOPTION_PARTIAL_ORDER_SERVICE_PROFILE = 10;       // RFC 1693, LENGTH: 3 Bytes
//  TCPOPTION_CC = 11;                                  // RFC 1644, LENGTH: -
//  TCPOPTION_CC_NEW = 12;                              // RFC 1644, LENGTH: -
//  TCPOPTION_CC_ECHO = 13;                             // RFC 1644, LENGTH: -
//  TCPOPTION_TCP_ALTERNATE_CHECKSUM_REQUEST = 14;      // RFC 1146, LENGTH: 3 Bytes
//  TCPOPTION_TCP_ALTERNATE_CHECKSUM_DATA = 15;         // RFC 1146, LENGTH: N
//  TCPOPTION_SKEETER = 16;                             // [Knowles], LENGTH: -
//  TCPOPTION_BUBBA = 17;                               // [Knowles], LENGTH: -
//  TCPOPTION_TRAILER_CHECKSUM_OPTION = 18;             // [Subbu & Monroe], LENGTH: 3Bytes
//     TCPOPTION_MD5_SIGNATURE_OPTION = 19;             // (obsoleted by option 29) RFC 2385, LENGTH: 18 Bytes
//  TCPOPTION_SCPS_CAPABILITIES = 20;                   // [Scott], LENGTH: -
//  TCPOPTION_SELECTIVE_NEGATIVE_ACKNOWLEDGEMENTS = 21; // [Scott], LENGTH: -
//  TCPOPTION_RECORD_BOUNDARIES = 22;                   // [Scott], LENGTH: -
//  TCPOPTION_CORRUPTION_EXPERIENCED = 23;              // [Scott], LENGTH: -
//  TCPOPTION_SANP = 24;                                // [Sukonnik], LENGTH: -
//  TCPOPTION_UNASSIGNED = 25;                          // released 2000-12-18 [-], LENGTH: -
//  TCPOPTION_TCP_COMPESSION_FILTER = 26;               // [Bellovin], LENGTH: -
//  TCPOPTION_QUICK_START_RESPONSE = 27;                // RFC 4782, LENGTH: 8 Bytes
//  TCPOPTION_USER_TIMEOUT_OPTION = 28;                 // RFC 5482, LENGTH: 4 Bytes
//  TCPOPTION_AUTHENTICATION_OPTION = 29;               // RFC 5925, LENGTH: -
//    TCPOPTION kinds 30-252 Unassigned
//  TCPOPTION_RFC3692_STYLE_EXPERIMENT_1 = 253;         // RFC 4727, LENGTH: N
//  TCPOPTION_RFC3692_STYLE_EXPERIMENT_2 = 254;         // RFC 4727, LENGTH: N
};

//
// This structure represents a single SACK (selective acknowledgment):
//
class Sack
{
    unsigned int start;     // start seq no. of sack block
    unsigned int end;       // end seq no. of sack block
}

// Header Options (optional):
class TCPOption
{
    unsigned short kind enum(TCPOptionNumbers) = TCPOPTION_END_OF_OPTION_LIST;  // option kind
    unsigned short length = 1;                          // option length
    unsigned int values[];                              // option value(s)
}

//
// Represents a \TCP segment, to be used with the TCP module.
//
// \TCP header fields not explicitly modelled: work on going
// - Data Offset (number of 32 bit words in the header): represented
//   by cMessage::length().
// - Reserved (reserved for future use)
// - Checksum (header checksum): modelled by cMessage::hasBitError()
// - Header Options: Currently only EOL, NOP, MSS, WS, SACK_PERMITTED, SACK and TS are implemented
// - Padding
//
// cMessage::getKind() may be set to an arbitrary value: TCP entities will
// ignore it and use only the header fields (synBit, ackBit, rstBit).
//
packet TCPSegment
{
    @customize(true);
    // Source Port
    unsigned short srcPort;

    // Destination Port
    unsigned short destPort;

    // Sequence Number: first sequence number of the first data octet
    // in the respective segment (except if SYN is set; then the the
    // seq. number is the initial seq. number (ISS) and the first data
    // octet is ISS+1)
    unsigned int sequenceNo;

    // Acknowledgement Number: if ACK flag is set, this field contains
    // the next sequence number the sender of this segment is expecting
    // to receive
    unsigned int ackNo;

    // TCP Header Length - default: 20 bytes
    // if header options are used the headerLength is greater then 20 bytes (default)
    unsigned short headerLength = TCP_HEADER_OCTETS; // TCP_HEADER_OCTETS = 20

    bool urgBit; // URG: urgent pointer field significant if set
    bool ackBit; // ACK: ackNo significant if set
    bool pshBit; // PSH: push function
    bool rstBit; // RST: reset the connection
    bool synBit; // SYN: synchronize seq. numbers
    bool finBit; // FIN: finish - no more data from sender

    // Window Size: the number of data octets beginning with the one indicated
    // in the acknowledgement field which the sender of this segment is
    // willing to accept
    unsigned short window;

    // Urgent Pointer: communicates the current value of the urgent pointer
    // as a positive offset from the sequence number in this segment. The
    // urgent pointer points to the sequence number of the octet following
    // the urgent data. This field is only be interpreted in segments with
    // the URG control bit set.
    unsigned short urgentPointer;

    // Header options (optional)
    // Currently only EOL, NOP, MSS, WS, SACK_PERMITTED, SACK and TS are implemented
    TCPOption options[];

    // Payload length in octets (not an actual \TCP header field).
    // This may not always be the same as encapsulatedPacket()->getByteLength();
    // e.g. when simulating a virtual data stream there's no encapsulated
    // packet at all.
    unsigned long payloadLength;

    // Message objects (cMessages) that travel in this segment as data.
    // (This field is used only with TCPMsgBasedSendQueue/RcvQueue and
    // not with TCPVirtualBytesSendQueue/RcvQueue.)  Every message object
    // is put into the TCPSegment that would (in real life) carry its
    // last octet. That is, if message object 'msg' with length=100 bytes
    // occupies sequence number range 10000..10099, it will travel in the
    // TCPSegment which carries the octet 10099. This way it is easily achieved
    // that the receiving TCP passes up the message object to its client
    // when the last byte of the message has arrived.
    abstract TCPPayloadMessage payload[];
}
