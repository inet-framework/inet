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
#include "TCPSegment.h"
#include "ByteArray.h"

    typedef cPacket *cPacketPtr;

    inline std::ostream& operator<<(std::ostream& os, cPacketPtr msg)
    {
        return os << "(" << msg->getClassName() << ") " << msg->getName() << ": " << msg->info();
    }

    inline void doPacking(cCommBuffer *b, cPacketPtr& msg) {msg->parsimPack(b);}
    inline void doUnpacking(cCommBuffer *b, cPacketPtr& msg) {msg->parsimUnpack(b);}
}}

packet TCPSegment;

struct cPacketPtr;

class noncobject ByteArray;

struct TCPPayloadMessage
{
    uint64 streamOffs;   // offset of first  byte of msg in the full tcp stream
    uint64 segmentOffs;  // offset of first  byte of msg in the current tcp segment
    cPacketPtr msg;      // pointer to payload msg
}

packet TCPSegmentWithMessages extends TCPSegment
{
    @customize(true);

    // Message objects (cMessages) that travel in this segment as data.
    // (This field is used only with TCPDataTransferMode==TCP_TRANSFER_OBJECT)
    // Every message object is put into the TCPSegment that would (in real life)
    // carry its first octet. That is, if message object 'msg' with length=100 bytes
    // occupies stream offset number range 10000..10099, it will travel in the
    // TCPSegment which carries the octet 10000. This way it is easily achieved
    // that the receiving TCP passes up the message object to its client
    // when the last byte of the message has arrived.
    abstract TCPPayloadMessage payload[];
}

packet TCPSegmentWithBytes extends TCPSegment
{
    @customize(true);

    // Message bytes that travel in this segment as data.
    // (This field is used only with TCPDataTransferMode==TCP_TRANSFER_BYTECOUNT)
    ByteArray byteArray;
}
