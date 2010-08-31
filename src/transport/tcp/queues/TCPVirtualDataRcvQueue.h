//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009 Thomas Reschka
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

#ifndef __INET_TCPVIRTUALDATARCVQUEUE_H
#define __INET_TCPVIRTUALDATARCVQUEUE_H

#include <list>
#include <string>
#include "TCPSegment.h"
#include "TCPReceiveQueue.h"

/**
 * Receive queue that manages "virtual bytes", that is, byte counts only.
 *
 * @see TCPVirtualDataSendQueue
 */
class INET_API TCPVirtualDataRcvQueue : public TCPReceiveQueue
{
  protected:
    uint32 rcv_nxt;

    class Region
    {
      protected:
        uint32 begin;
        uint32 end;
      public:
        enum CompareStatus {BEFORE=1, BEFORE_TOUCH, OVERLAP, AFTER_TOUCH, AFTER };
        Region(uint32 _begin, uint32 _end) : begin(_begin),end(_end) {};
        virtual ~Region() {};
        uint32 getBegin() const {return begin;}
        uint32 getEnd() const {return end;}
        unsigned long getLength() const {return (ulong)(end - begin);}
        unsigned long getLengthTo(uint32 seq) const;

        /// Compare self and other
        CompareStatus compare(const TCPVirtualDataRcvQueue::Region& other) const;

        // Virtual functions:

        /// Merge other region to self
        virtual bool merge(const TCPVirtualDataRcvQueue::Region* other);

        /// Copy self to msg
        virtual void copyTo(TCPDataMsg* msg) const;

        /**
         * Returns an allocated new Region object with filled with begin..seq and set self to seq..end
         */
        virtual TCPVirtualDataRcvQueue::Region* split(uint32 seq);
    };
    typedef std::list<Region*> RegionList;
    RegionList regionList;

    /// Merge segment byte range into regionList, the parameter region must created by 'new' operator.
    void merge(TCPVirtualDataRcvQueue::Region *region);

    /// returns the number of bytes extracted
    Region* extractTo(uint32 toSeq, ulong maxBytes);

  public:
    /**
     * Ctor.
     */
    TCPVirtualDataRcvQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPVirtualDataRcvQueue();

    /**
     * Set initial receive sequence number.
     */
    virtual void init(uint32 startSeq);

    void setConnection(TCPConnection *_conn);

    /**
     * Returns a string with region stored.
     */
    virtual std::string info() const;

    /**
     * Called when a TCP segment arrives. Returns sequence number for ACK.
     */
    virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg);

    virtual uint32 insertBytesFromRegion(TCPVirtualDataRcvQueue::Region *region);

    /**
     *
     */
    virtual ulong getExtractableBytesUpTo(uint32 seq);

    /**
     *
     */
    virtual TCPDataMsg* extractBytesUpTo(uint32 seq, ulong maxBytes);

    /**
     * Returns the number of bytes (out-of-order-segments) currently buffered in queue.
     */
    virtual uint32 getAmountOfBufferedBytes();

    /**
     * Returns the number of bytes currently free (=available) in queue. freeRcvBuffer = maxRcvBuffer - usedRcvBuffer
     */
    virtual uint32 getAmountOfFreeBytes(uint32 maxRcvBuffer);

    /**
     *
     */
    virtual uint32 getQueueLength();

    /**
     *
     */
    virtual void getQueueStatus();

    /**
     *
     */
    virtual uint32 getLE(uint32 fromSeqNum);

    /**
     *
     */
    virtual uint32 getRE(uint32 toSeqNum);
};

#endif
