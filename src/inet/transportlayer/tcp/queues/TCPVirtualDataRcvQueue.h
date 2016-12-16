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

#include "inet/common/packet/ReorderBuffer.h"
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
    ReorderBuffer reorderBuffer;

    uint32_t offsetToSeq(int64_t offs) const { return (uint32_t)offs; }

    int64_t seqToOffset(uint32_t seq) const
    {
        int64_t expOffs = reorderBuffer.getExpectedOffset();
        uint32_t expSeq = offsetToSeq(expOffs);
        return (seqGE(seq, expSeq)) ? expOffs + (seq - expSeq) : expOffs - (expSeq - seq);
    }

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
    virtual uint32 insertBytesFromSegment(Packet *packet, TcpHeader *tcpseg) override;

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

