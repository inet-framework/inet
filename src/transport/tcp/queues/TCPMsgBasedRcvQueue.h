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

#ifndef __INET_TCPMESSAGERCVQUEUE_H
#define __INET_TCPMESSAGERCVQUEUE_H

#include <map>
#include <string>
#include <omnetpp.h>
#include "TCPSegment.h"
#include "TCPVirtualDataRcvQueue.h"

/**
 * FIXME
 *
 * @see TCPMsgBasedSendQueue
 */
class INET_API TCPMsgBasedRcvQueue : public TCPVirtualDataRcvQueue
{
  protected:
    typedef std::map<uint64, cPacket *> PayloadList;
    PayloadList payloadList;
    uint64 extractedBytes;
    uint64 extractedPayloadBytes;
    bool isPayloadExtractAtFirst;

  public:
    /**
     * Ctor.
     */
    TCPMsgBasedRcvQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPMsgBasedRcvQueue();

    virtual void setConnection(TCPConnection *_conn);

    /**
     * Set initial receive sequence number.
     */
    virtual void init(uint32 startSeq);

    /**
     * Returns a string with region stored.
     */
    virtual std::string info() const;

    /**
     * Called when a TCP segment arrives. Returns sequence number for ACK.
     */
    virtual uint32 insertBytesFromSegment(TCPSegment *tcpseg);

    /**
     *
     */
    virtual TCPDataMsg* extractBytesUpTo(uint32 seq, ulong maxBytes);

};

#endif

