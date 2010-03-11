//
// Copyright (C) 2005-2009 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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

#ifndef __SCTPQUEUE_H
#define __SCTPQUEUE_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "SCTP.h"


class SCTPDataVariables;
class SCTPAssociation;


/**
 * Abstract base class for SCTP receive queues. This class represents
 * data received by SCTP but not yet passed up to the application.
 * The class also accomodates for selective retransmission, i.e.
 * also acts as a segment buffer.
 *
 * This class goes hand-in-hand with SCTPSendQueue.
 *
 * This class is polymorphic because depending on where and how you
 * use the SCTP model you might have different ideas about "sending data"
 * on a simulated connection: you might want to transmit real bytes,
 * "dummy" (byte count only), cMessage objects, etc; see discussion
 * at SCTPSendQueue. Different subclasses can be written to accomodate
 * different needs.
 *
 * @see SCTPSendQueue
 */
class INET_API SCTPQueue : public cPolymorphic
{
    public:
    /**
      * Constructor.
      */
     SCTPQueue();

     /**
      * Virtual destructor.
      */
     ~SCTPQueue();

    bool checkAndInsertChunk(const uint32 key, SCTPDataVariables* chunk);
    /* returns true if new data is inserted and false if data was present */

    SCTPDataVariables* getAndExtractChunk(const uint32 tsn);
    SCTPDataVariables* extractMessage();

    void printQueue() const;

    uint32 getQueueSize() const;

    SCTPDataVariables* getFirstChunk() const;

    cMessage* getMsg(const uint32 key) const;

    SCTPDataVariables* getChunk(const uint32 key) const;

    SCTPDataVariables* getChunkFast(const uint32 tsn, bool& firstTime);

    void removeMsg(const uint32 key);

    bool deleteMsg(const uint32 tsn);

    int32 getNumBytes() const;

    SCTPDataVariables* dequeueChunkBySSN(const uint16 ssn);


  public:
     typedef std::map<uint32, SCTPDataVariables*> PayloadQueue;
     PayloadQueue payloadQueue;

  protected:
     SCTPAssociation* assoc;    // SCTP connection object

  private:
     PayloadQueue::iterator GetChunkFastIterator;
};

#endif
