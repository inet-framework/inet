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


#ifndef __INET_TCPSEGMENTWITHDATA_H
#define __INET_TCPSEGMENTWITHDATA_H

#include <list>
#include "INETDefs.h"
#include "TCPSegmentWithData_m.h"


class INET_API TCPSegmentWithMessages : public TCPSegmentWithMessages_Base
{
  protected:
    typedef std::list<TCPPayloadMessage> PayloadList;
    PayloadList payloadList;

  public:
    TCPSegmentWithMessages(const char *name=NULL, int kind=0) : TCPSegmentWithMessages_Base(name,kind) {}
    TCPSegmentWithMessages(const TCPSegmentWithMessages& other) : TCPSegmentWithMessages_Base(other.getName()) {operator=(other);}
    virtual ~TCPSegmentWithMessages();
    TCPSegmentWithMessages& operator=(const TCPSegmentWithMessages& other);
    virtual TCPSegmentWithMessages *dup() const {return new TCPSegmentWithMessages(*this);}
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
     * Adds a message object to the TCP segment.
     * The stream offset number of the first byte of the message should be passed as 2nd argument.
     * The segment offset number of the first byte of the message should be passed as 3th argument.
     */
    virtual void addPayloadMessage(cPacket *msg, uint64 streamOffs, uint64 segmentOffs);

    /**
     * Removes and returns the first message object in this TCP segment.
     * It also returns the stream offset number of its first octet in streamOffs.
     * It also returns the segment offset number of its first octet in segmentOffs.
     */
    virtual cPacket *removeFirstPayloadMessage(uint64& streamOffs, uint64& segmentOffs);

  protected:
    /**
     * Truncate segment data.
     * @param truncleft: number of bytes for truncate from begin of data
     * @param truncright: number of bytes for truncate from end of data
     */
    virtual void truncateData(unsigned int truncleft, unsigned int truncright);
};

class INET_API TCPSegmentWithBytes : public TCPSegmentWithBytes_Base
{
  public:

    TCPSegmentWithBytes(const char *name=NULL, int kind=0) : TCPSegmentWithBytes_Base(name,kind) {}
    TCPSegmentWithBytes(const TCPSegmentWithBytes& other) : TCPSegmentWithBytes_Base(other.getName()) {operator=(other);}
    TCPSegmentWithBytes& operator=(const TCPSegmentWithBytes& other) {TCPSegmentWithBytes_Base::operator=(other); return *this;}
    virtual ~TCPSegmentWithBytes();

    virtual TCPSegmentWithBytes *dup() const {return new TCPSegmentWithBytes(*this);}

  protected:
    /**
     * Truncate segment data.
     * @param truncleft: number of bytes for truncate from begin of data
     * @param truncright: number of bytes for truncate from end of data
     */
    virtual void truncateData(unsigned int truncleft, unsigned int truncright);
};

#endif //__INET_TCPSEGMENTWITHDATA_H
