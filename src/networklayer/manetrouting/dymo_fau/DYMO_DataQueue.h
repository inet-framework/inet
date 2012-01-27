/*
 * Copyright (C) 2006 Christoph Sommer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DYMO_DATAQUEUE_H
#define DYMO_DATAQUEUE_H

#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#include "INETDefs.h"

#include "IPv4Address.h"
#include "IPv4ControlInfo_m.h"
#include "IPv4Datagram.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "IPv4.h"

class DYMO;
enum Result {DROP, ACCEPT};


class DYMO_QueuedData
{
  public:
    DYMO_QueuedData(IPv4Datagram* dgram, IPv4Address destAddr) : destAddr(destAddr) {datagram = dgram;}

    IPv4Datagram* datagram;
    IPv4Address destAddr;

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_QueuedData& o);
};

/**
 * Stores datagrams awaiting route discovery
 */
class DYMO_DataQueue : public cObject
{
  public:
    DYMO_DataQueue(cSimpleModule *owner, int BUFFER_SIZE_PACKETS, int BUFFER_SIZE_BYTES);
    ~DYMO_DataQueue();

    /** @brief inherited from cObject */
    virtual const char* getFullName() const;

    /** @brief inherited from cObject */
    virtual std::string info() const;

    /** @brief inherited from cObject */
    virtual std::string detailedInfo() const;

    void queuePacket(const IPv4Datagram* datagram);

    void dequeuePacketsTo(IPv4Address destAddr, int prefix);
    void dropPacketsTo(IPv4Address destAddr, int prefix, std::list<IPv4Datagram*>* datagrams = NULL);

  protected:
    cSimpleModule *moduleOwner;
    std::list<DYMO_QueuedData> dataQueue; /**< queued data packets */
    long dataQueueByteSize; /**< total size of all messages currently in dataQueue */

    int BUFFER_SIZE_PACKETS; /**< NED configuration parameter: maximum number of queued packets, -1 for no limit */
    int BUFFER_SIZE_BYTES; /**< NED configuration parameter: maximum total size of queued packets, -1 for no limit */

    void reinjectDatagramsTo(IPv4Address destAddr, int prefix, Result verdict, std::list<IPv4Datagram*> *datagrams = NULL);

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_DataQueue& o);
};

#endif

