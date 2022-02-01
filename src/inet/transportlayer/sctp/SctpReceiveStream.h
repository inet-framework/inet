//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

