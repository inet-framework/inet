//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPRECEIVESTREAM_H
#define __INET_SCTPRECEIVESTREAM_H

#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {
namespace sctp {

class INET_API SctpReceiveStream : public cObject
{
  protected:
    SctpAssociation *assoc;
    uint16_t streamId;
    int32_t expectedStreamSeqNum;
    SctpQueue *deliveryQ;
    SctpQueue *orderedQ;
    SctpQueue *unorderedQ;
    uint32_t reassemble(SctpQueue *queue, uint32_t tsn);

  public:
    uint32_t enqueueNewDataChunk(SctpDataVariables *dchunk);
    /**
     * Ctor.
     */
    SctpReceiveStream(SctpAssociation *assoc);
    int32_t getExpectedStreamSeqNum();
    void setExpectedStreamSeqNum(int32_t num);

    /**
     * Virtual dtor.
     */
    ~SctpReceiveStream();
    SctpQueue *getDeliveryQ() const { return deliveryQ; };
    SctpQueue *getOrderedQ() const { return orderedQ; };
    SctpQueue *getUnorderedQ() const { return unorderedQ; };

    int32_t getStreamId() const { return streamId; };
    void setStreamId(const uint16_t id) { streamId = id; }
};

} // namespace sctp
} // namespace inet

#endif

