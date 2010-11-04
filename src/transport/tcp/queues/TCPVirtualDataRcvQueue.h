//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009 Thomas Reschka
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

    struct Region
    {
        uint32 begin;
        uint32 end;
    };
    typedef std::list<Region> RegionList;
    RegionList regionList;

    // merges segment byte range into regionList
    void merge(uint32 segmentBegin, uint32 segmentEnd);

    // returns number of bytes extracted
    ulong extractTo(uint32 toSeq);

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

    /**
     * Returns a string with region stored.
     */
    virtual std::string info() const;

    /**
     * Called when a TCP segment arrives. Returns sequence number for ACK.
     */
    virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg);

    /**
     *
     */
    virtual cPacket *extractBytesUpTo(uint32 seq);

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
