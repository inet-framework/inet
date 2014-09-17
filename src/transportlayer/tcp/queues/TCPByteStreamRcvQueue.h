//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPBYTESTREAMRCVQUEUE_H
#define __INET_TCPBYTESTREAMRCVQUEUE_H

#include <map>
#include <string>

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/transportlayer/tcp/queues/TCPVirtualDataRcvQueue.h"
#include "inet/common/ByteArray.h"

namespace inet {

namespace tcp {

/**
 * TCP send queue that stores actual bytes.
 *
 * @see TCPSendQueue
 */
class INET_API TCPByteStreamRcvQueue : public TCPVirtualDataRcvQueue
{
  protected:
    class Region : public TCPVirtualDataRcvQueue::Region
    {
      protected:
        ByteArray data;

      public:
        Region(uint32 _begin, uint32 _end) : TCPVirtualDataRcvQueue::Region(_begin, _end) {};
        Region(uint32 _begin, uint32 _end, ByteArray _data)
            : TCPVirtualDataRcvQueue::Region(_begin, _end), data(_data) {};

        virtual ~Region() {};

        /// Merge other to self
        virtual bool merge(const TCPVirtualDataRcvQueue::Region *other);

        /// Copy self to msg
        virtual void copyTo(cPacket *msg) const;

        /**
         * Returns an allocated new Region object with filled with begin..seq and set self to seq..end
         */
        virtual TCPByteStreamRcvQueue::Region *split(uint32 seq);
    };

  public:
    /**
     * Ctor.
     */
    TCPByteStreamRcvQueue() : TCPVirtualDataRcvQueue() {};

    /**
     * Virtual dtor.
     */
    virtual ~TCPByteStreamRcvQueue();

    /**
     * Returns a string with region stored.
     */
    virtual std::string info() const;

    cPacket *extractBytesUpTo(uint32 seq);

    /**
     * Create a new Region from tcpseg.
     * Called from insertBytesFromSegment()
     */
    virtual TCPVirtualDataRcvQueue::Region *createRegionFromSegment(TCPSegment *tcpseg);
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPBYTESTREAMRCVQUEUE_H

