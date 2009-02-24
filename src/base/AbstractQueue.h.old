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

#ifndef __FIFOBASE_H
#define __FIFOBASE_H

#include <omnetpp.h>
#include "INETDefs.h"


/**
 * Abstract base class for single-server queues.
 */
class INET_API AbstractQueue : public cSimpleModule
{
  public:
    AbstractQueue();
    virtual ~AbstractQueue();

  protected:
    cMessage *msgBeingServiced;
    cMessage *endServiceTimer;

  protected:
    virtual void doStartService(cMessage *msg);
    virtual void doEndService(cMessage *msg);

  protected:
    /**
     * The queue.
     */
    cPacketQueue queue;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    /** Functions to (re)define behaviour */

    //@{
    /**
     * Called when a message arrives at the module. The method should
     * either enqueue this message (usual behaviour), or discard it.
     * It may also wrap the it into another message, and insert that one
     * in the queue.
     *
     * Most straightforward implementation: <tt>queue.insert(msg);</tt>
     */
    virtual void arrival(cMessage *msg) = 0;

    /**
     * Called when a message arrives at the module when the queue is
     * empty. The message doesn't need to be enqueued in this case,
     * it can start service immmediately. This method may:
     *  -# simply return the the same pointer (usual behaviour), or
     *  -# discard the message and return NULL pointer (the effect being
     *   this message being ignored)
     *  -# or modify the message, wrap in into another message etc, and
     *   return the (new) message's pointer.
     *
     * Most straightforward implementation: <tt>return msg;</tt>
     */
    virtual cMessage *arrivalWhenIdle(cMessage *msg) = 0;

    /**
     * Called when a message starts service, and should return the service time.
     *
     * Example implementation: <tt>return 1.0;</tt>
     */
    virtual simtime_t startService(cMessage *msg) = 0;

    /**
     * Called when a message completes service. The function may send it
     * to another module, discard it, or in general do anything with it.
     *
     * Most straightforward implementation: <tt>send(msg,"out");</tt>
     */
    virtual void endService(cMessage *msg) = 0;
    //@}
};

#endif


