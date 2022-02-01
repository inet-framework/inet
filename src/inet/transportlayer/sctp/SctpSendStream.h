//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPSENDSTREAM_H
#define __INET_SCTPSENDSTREAM_H

#include <list>

#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {

class SctpCommand;

namespace sctp {

class SctpHeader;
class SctpDataVariables;

class INET_API SctpSendStream : public cObject
{
  protected:
    SctpAssociation *assoc;
    uint16_t streamId;
    uint16_t nextStreamSeqNum;
    uint32_t bytesInFlight;
    bool resetRequested;
    bool fragInProgress;
    cPacketQueue *streamQ;
    cPacketQueue *uStreamQ;

  public:

    SctpSendStream(SctpAssociation *assoc, const uint16_t id);
    ~SctpSendStream();

    cPacketQueue *getStreamQ() const { return streamQ; };
    cPacketQueue *getUnorderedStreamQ() const { return uStreamQ; };
    uint32_t getNextStreamSeqNum();
    uint32_t getBytesInFlight() const { return bytesInFlight; };
    void setNextStreamSeqNum(const uint16_t num);
    void setBytesInFlight(const uint32_t bytes) { bytesInFlight = bytes; }
    bool getFragInProgress() const { return fragInProgress; };
    void setFragInProgress(const bool frag) { fragInProgress = frag; }
    uint16_t getStreamId() const { return streamId; };
    void setStreamId(const uint16_t id) { streamId = id; }
    void deleteQueue();
};

} // namespace sctp
} // namespace inet

#endif

