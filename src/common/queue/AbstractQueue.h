//
// Copyright (C) 2004, 2008 Andras Varga
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

#ifndef __INET_ABSTRACTQUEUE_H
#define __INET_ABSTRACTQUEUE_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Abstract base class for single-server queues. Contains special
 * optimization for zero service time (i.e. it does not schedule the
 * endService timer then).
 */
class INET_API AbstractQueue : public cSimpleModule
{
  public:
    AbstractQueue();
    virtual ~AbstractQueue();

  private:
    cPacket *msgServiced;
    cMessage *endServiceMsg;

  private:
    void doStartService();
    void doEndService();

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
    virtual void arrival(cPacket *msg) = 0;

    /**
     * Called when a message arrives at the module when the queue is
     * empty. The message doesn't need to be enqueued in this case,
     * it can start service immediately. This method may:
     *  -# simply return the the same pointer (usual behaviour), or
     *  -# discard the message and return NULL pointer (the effect being
     *   this message being ignored)
     *  -# or modify the message, wrap in into another message etc, and
     *   return the (new) message's pointer.
     *
     * Most straightforward implementation: <tt>return msg;</tt>
     */
    virtual cPacket *arrivalWhenIdle(cPacket *msg) = 0;

    /**
     * Called when a message starts service, and should return the service time.
     *
     * Example implementation: <tt>return 1.0;</tt>
     */
    virtual simtime_t startService(cPacket *msg) = 0;

    /**
     * Called when a message completes service. The function may send it
     * to another module, discard it, or in general do anything with it.
     *
     * Most straightforward implementation: <tt>send(msg,"out");</tt>
     */
    virtual void endService(cPacket *msg) = 0;

    /**
     * If a message is under service, aborts its service and returns the
     * message. Returns NULL if no message is being serviced. The caller
     * is free to delete the message, reinsert it into the queue, or handle
     * it otherwise.
     */
    virtual cPacket *cancelService();
    //@}
};

} // namespace inet

#endif // ifndef __INET_ABSTRACTQUEUE_H

