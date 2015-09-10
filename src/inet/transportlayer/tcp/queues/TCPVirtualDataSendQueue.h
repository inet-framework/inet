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

#ifndef __INET_TCPVIRTUALDATASENDQUEUE_H
#define __INET_TCPVIRTUALDATASENDQUEUE_H

#include "inet/transportlayer/tcp/TCPSendQueue.h"

namespace inet {

namespace tcp {

/**
 * Send queue that manages "virtual bytes", that is, byte counts only.
 * It uses the length() field of the messages only, everything else is
 * ignored.
 *
 * @see TCPVirtualDataRcvQueue
 */
class INET_API TCPVirtualDataSendQueue : public TCPSendQueue
{
  protected:
    uint32 begin;    // 1st sequence number stored
    uint32 end;    // last sequence number stored +1

  public:
    /**
     * Ctor
     */
    TCPVirtualDataSendQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPVirtualDataSendQueue();

    /**
     *
     */
    virtual void init(uint32 startSeq) override;

    /**
     * Returns a string with the region stored.
     */
    virtual std::string info() const override;

    /**
     *
     */
    virtual void enqueueAppData(cPacket *msg) override;

    /**
     *
     */
    virtual uint32 getBufferStartSeq() override;

    /**
     *
     */
    virtual uint32 getBufferEndSeq() override;

    /**
     *
     */
    virtual TCPSegment *createSegmentWithBytes(uint32 fromSeq, ulong numBytes) override;

    /**
     *
     */
    virtual void discardUpTo(uint32 seqNum) override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPVIRTUALDATASENDQUEUE_H

