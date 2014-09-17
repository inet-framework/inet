//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPRECEIVESTREAM_H
#define __INET_SCTPRECEIVESTREAM_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/sctp/SCTPQueue.h"

namespace inet {

namespace sctp {

class INET_API SCTPReceiveStream : public cObject
{
  protected:
    uint16 streamId;
    int32 expectedStreamSeqNum;
    SCTPQueue *deliveryQ;
    SCTPQueue *orderedQ;
    SCTPQueue *unorderedQ;
    uint32 reassemble(SCTPQueue *queue, uint32 tsn);

  public:
    uint32 enqueueNewDataChunk(SCTPDataVariables *dchunk);
    /**
     * Ctor.
     */
    SCTPReceiveStream();

    /**
     * Virtual dtor.
     */
    ~SCTPReceiveStream();
    inline SCTPQueue *getDeliveryQ() const { return deliveryQ; };
    inline SCTPQueue *getOrderedQ() const { return orderedQ; };
    inline SCTPQueue *getUnorderedQ() const { return unorderedQ; };
    inline int32 getExpectedStreamSeqNum() const { return expectedStreamSeqNum; };
    inline int32 getStreamId() const { return streamId; };
    inline void setExpectedStreamSeqNum(const int32 num) { expectedStreamSeqNum = num; };
    inline void setStreamId(const uint16 id) { streamId = id; };
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPRECEIVESTREAM_H

