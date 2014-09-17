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

#ifndef __INET_SCTPSENDSTREAM_H
#define __INET_SCTPSENDSTREAM_H

#include <list>

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/sctp/SCTPQueue.h"

namespace inet {

class SCTPCommand;

namespace sctp {

class SCTPMessage;
class SCTPDataVariables;

class INET_API SCTPSendStream : public cObject
{
  protected:
    uint16 streamId;
    uint16 nextStreamSeqNum;
    cPacketQueue *streamQ;
    cPacketQueue *uStreamQ;
    int32 ssn;
    uint64 totalLength;

  public:

    SCTPSendStream(const uint16 id);
    ~SCTPSendStream();

    inline cPacketQueue *getStreamQ() const { return streamQ; };
    inline cPacketQueue *getUnorderedStreamQ() const { return uStreamQ; };
    inline uint32 getNextStreamSeqNum() const { return nextStreamSeqNum; };
    inline void setNextStreamSeqNum(const uint16 num) { nextStreamSeqNum = num; };
    inline uint16 getStreamId() const { return streamId; };
    inline void setStreamId(const uint16 id) { streamId = id; };
    void deleteQueue();
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTPSENDSTREAM_H

