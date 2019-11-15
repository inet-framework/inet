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
#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {
namespace sctp {

class INET_API SctpReceiveStream : public cObject
{
  protected:
    SctpAssociation *assoc;
    uint16 streamId;
    int32 expectedStreamSeqNum;
    SctpQueue *deliveryQ;
    SctpQueue *orderedQ;
    SctpQueue *unorderedQ;
    uint32 reassemble(SctpQueue *queue, uint32 tsn);

  public:
    uint32 enqueueNewDataChunk(SctpDataVariables *dchunk);
    /**
     * Ctor.
     */
    SctpReceiveStream(SctpAssociation *assoc);
    int32 getExpectedStreamSeqNum();
    void setExpectedStreamSeqNum(int32 num);

    /**
     * Virtual dtor.
     */
    ~SctpReceiveStream();
    SctpQueue *getDeliveryQ() const { return deliveryQ; };
    SctpQueue *getOrderedQ() const { return orderedQ; };
    SctpQueue *getUnorderedQ() const { return unorderedQ; };

    int32 getStreamId() const { return streamId; };
    void setStreamId(const uint16 id) { streamId = id; };
};

} // namespace sctp
} // namespace inet

#endif // ifndef __INET_SCTPRECEIVESTREAM_H

