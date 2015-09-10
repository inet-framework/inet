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

#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/tcp/TCPReceiveQueue.h"

namespace inet {

namespace tcp {

/**
 * Receive queue that manages "virtual bytes", that is, byte counts only.
 *
 * @see TCPVirtualDataSendQueue
 */
class INET_API TCPVirtualDataRcvQueue : public TCPReceiveQueue
{
  protected:
    uint32 rcv_nxt = 0;

    class Region
    {
      protected:
        uint32 begin;
        uint32 end;

      public:
        enum CompareStatus { BEFORE = 1, BEFORE_TOUCH, OVERLAP, AFTER_TOUCH, AFTER };
        Region(uint32 _begin, uint32 _end) : begin(_begin), end(_end) {};
        virtual ~Region() {};
        uint32 getBegin() const { return begin; }
        uint32 getEnd() const { return end; }
        unsigned long getLength() const { return (ulong)(end - begin); }
        unsigned long getLengthTo(uint32 seq) const;

        /** Compare self and other */
        CompareStatus compare(const TCPVirtualDataRcvQueue::Region& other) const;

        // Virtual functions:

        /** Merge other region to self */
        virtual bool merge(const TCPVirtualDataRcvQueue::Region *other);

        /** Copy self to msg */
        virtual void copyTo(cPacket *msg) const;

        /**
         * Returns an allocated new Region object with filled with [begin..seq) and set self to [seq..end)
         */
        virtual TCPVirtualDataRcvQueue::Region *split(uint32 seq);
    };

    typedef std::list<Region *> RegionList;

    RegionList regionList;

    /** Merge segment byte range into regionList, the parameter region must created by 'new' operator. */
    void merge(TCPVirtualDataRcvQueue::Region *region);

    // Returns number of bytes extracted
    TCPVirtualDataRcvQueue::Region *extractTo(uint32 toSeq);

    /**
     * Create a new Region from tcpseg.
     * Called from insertBytesFromSegment()
     */
    virtual TCPVirtualDataRcvQueue::Region *createRegionFromSegment(TCPSegment *tcpseg);

  public:
    /**
     * Ctor.
     */
    TCPVirtualDataRcvQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPVirtualDataRcvQueue();

    /** Method inherited from TCPReceiveQueue */
    virtual void init(uint32 startSeq) override;

    /** Method inherited from TCPReceiveQueue */
    virtual std::string info() const override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg) override;

    /** Method inherited from TCPReceiveQueue */
    virtual cPacket *extractBytesUpTo(uint32 seq) override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 getAmountOfBufferedBytes() override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 getAmountOfFreeBytes(uint32 maxRcvBuffer) override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 getQueueLength() override;

    /** Method inherited from TCPReceiveQueue */
    virtual void getQueueStatus() override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 getLE(uint32 fromSeqNum) override;

    /** Method inherited from TCPReceiveQueue */
    virtual uint32 getRE(uint32 toSeqNum) override;

    virtual uint32 getFirstSeqNo() override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPVIRTUALDATARCVQUEUE_H

