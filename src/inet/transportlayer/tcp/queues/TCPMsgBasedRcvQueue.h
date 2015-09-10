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

#ifndef __INET_TCPMSGBASEDRCVQUEUE_H
#define __INET_TCPMSGBASEDRCVQUEUE_H

#include <map>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/tcp/queues/TCPVirtualDataRcvQueue.h"

namespace inet {

namespace tcp {

/**
 * @see TCPMsgBasedSendQueue
 */
class INET_API TCPMsgBasedRcvQueue : public TCPVirtualDataRcvQueue
{
  protected:
    struct PayloadItem
    {
        uint32 seqNo;
        cPacket *packet;
        PayloadItem(uint32 _seqNo, cPacket *_packet) : seqNo(_seqNo), packet(_packet) {}
    };
    typedef std::list<PayloadItem> PayloadList;
    PayloadList payloadList;    // sorted list, used the sequence number comparators

  public:
    /**
     * Ctor.
     */
    TCPMsgBasedRcvQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPMsgBasedRcvQueue();

    /**
     * Set initial receive sequence number.
     */
    virtual void init(uint32 startSeq) override;

    /**
     * Returns a string with region stored.
     */
    virtual std::string info() const override;

    /**
     * Called when a TCP segment arrives. Returns sequence number for ACK.
     */
    virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg) override;

    /**
     *
     */
    virtual cPacket *extractBytesUpTo(uint32 seq) override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPMSGBASEDRCVQUEUE_H

