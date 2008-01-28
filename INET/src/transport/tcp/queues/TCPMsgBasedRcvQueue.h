//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __TCPMESSAGERCVQUEUE_H
#define __TCPMESSAGERCVQUEUE_H

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
    typedef std::map<uint32, cMessage *> PayloadList;
    PayloadList payloadList;

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
    virtual cMessage *extractBytesUpTo(uint32 seq);

};

#endif

