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

#ifndef __TCPRECEIVEQUEUE_H
#define __TCPRECEIVEQUEUE_H

#include <omnetpp.h>
#include "TCPConnection.h"


class TCPSegment;
class TCPInterfacePacket;


/**
 * Abstract base class for TCP receive queues. This class represents
 * data received by TCP but not yet passed up to the application.
 * The class also accomodates for selective retransmission, i.e.
 * also acts as a segment buffer.
 *
 * This class goes hand-in-hand with TCPSendQueue.
 *
 * This class is polymorphic because depending on where and how you
 * use the TCP model you might have different ideas about "sending data"
 * on a simulated connection: you might want to transmit real bytes,
 * "dummy" (byte count only), cMessage objects, etc; see discussion
 * at TCPSendQueue. Different subclasses can be written to accomodate
 * different needs.
 *
 * @see TCPSendQueue
 */
class TCPReceiveQueue : public cPolymorphic
{
  protected:
    TCPConnection *conn; // TCP connection object

  public:
    /**
     * Ctor.
     */
    TCPReceiveQueue(TCPConnection *_conn)  {conn=_conn;}

    /**
     * Virtual dtor.
     */
    virtual ~TCPReceiveQueue() {}

    /**
     * Should be redefined to return whether selective schemes
     * are supported. (Code can be a lot simpler with the original
     * commulative retransmission).
     */
    virtual bool supportsSelectiveRetransmission() = 0;

    /**
     * Called when a TCP segment arrives.
     */
    virtual insertBytesFromSegment(TCPSegment *tcpseg) = 0;

    /**
     * Returns how many bytes are available in the queue for passing
     * up to the user. (That is, doesn't include bytes past the
     * first "hole" if selective retransmission is used.)
     */
    virtual ulong bytesAvailable() = 0;

    /**
     * Called on RECEIVE app command. Creates packet to be passed up the app.
     */
    virtual TCPInterfacePacket *extractBytes() = 0;

};

#endif


