//
// Copyright (C) 2004 Andras Varga
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


#ifndef __INET_TCPSEGMENT_H
#define __INET_TCPSEGMENT_H

#include <list>
#include "INETDefs.h"
#include "TCPSegment_m.h"


/** @name Comparing sequence numbers */
//@{
inline bool seqLess(uint32 a, uint32 b) {return a!=b && b-a<(1UL<<31);}
inline bool seqLE(uint32 a, uint32 b) {return b-a<(1UL<<31);}
inline bool seqGreater(uint32 a, uint32 b) {return a!=b && a-b<(1UL<<31);}
inline bool seqGE(uint32 a, uint32 b) {return a-b<(1UL<<31);}
//@}


/**
 * Represents a TCP segment. More info in the TCPSegment.msg file
 * (and the documentation generated from it).
 */
class INET_API TCPSegment : public TCPSegment_Base
{
  protected:
    std::list<TCPPayloadMessage> payloadList;

  public:
    TCPSegment(const char *name=NULL, int kind=0) : TCPSegment_Base(name,kind) {}
    TCPSegment(const TCPSegment& other) : TCPSegment_Base(other.getName()) {operator=(other);}
    virtual ~TCPSegment();
    TCPSegment& operator=(const TCPSegment& other);
    virtual TCPSegment *dup() const {return new TCPSegment(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    /** Generated but unused method, should not be called. */
    virtual void setPayloadArraySize(unsigned int size);
    /** Generated but unused method, should not be called. */
    virtual void setPayload(unsigned int k, const TCPPayloadMessage& payload_var);

    /**
     * Returns the number of payload messages in this TCP segment
     */
    virtual unsigned int getPayloadArraySize() const;

    /**
     * Returns the kth payload message in this TCP segment
     */
    virtual TCPPayloadMessage& getPayload(unsigned int k);

    /**
     * Adds a message object to the TCP segment. The sequence number+1 of the
     * last byte of the message should be passed as 2nd argument
     */
    virtual void addPayloadMessage(cPacket *msg, uint32 endSequenceNo);

    /**
     * Removes and returns the first message object in this TCP segment.
     * It also returns the sequence number+1 of its last octet in outEndSequenceNo.
     */
    virtual cPacket *removeFirstPayloadMessage(uint32& outEndSequenceNo);

    /**
     * Truncate segment.
     * @param firstSeqNo: sequence no of new first byte
     * @param endSeqNo: sequence no of new last byte+1
     */
    virtual void truncateSegment(uint32 firstSeqNo, uint32 endSeqNo);
};

#endif


