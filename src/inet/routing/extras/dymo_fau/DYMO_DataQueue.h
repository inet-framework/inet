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

#ifndef __INET_DYMO_DATAQUEUE_H
#define __INET_DYMO_DATAQUEUE_H

#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"

namespace inet {

namespace inetmanet {

class DYMOFau;
enum Result {DROP, ACCEPT};


class DYMO_QueuedData
{
  public:
    DYMO_QueuedData(Packet* dgram, L3Address destAddr) : destAddr(destAddr) {datagram = dgram;}

    Packet* datagram;
    L3Address destAddr;

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
    virtual const char* getFullName() const override;

    /** @brief inherited from cObject */
    virtual std::string str() const override;

    void queuePacket(Packet* datagram);

    void dequeuePacketsTo(L3Address destAddr, int prefix);
    void dropPacketsTo(L3Address destAddr, int prefix, std::list<Packet*>* datagrams = nullptr);

  protected:
    ManetRoutingBase *moduleOwner;
    std::list<DYMO_QueuedData> dataQueue; /**< queued data packets */
    long dataQueueByteSize; /**< total size of all messages currently in dataQueue */

    int BUFFER_SIZE_PACKETS; /**< NED configuration parameter: maximum number of queued packets, -1 for no limit */
    int BUFFER_SIZE_BYTES; /**< NED configuration parameter: maximum total size of queued packets, -1 for no limit */

    void reinjectDatagramsTo(L3Address destAddr, int prefix, Result verdict, std::list<Packet*> *datagrams = nullptr);

  public:
    friend std::ostream& operator<<(std::ostream& os, const DYMO_DataQueue& o);
};

} // namespace inetmanet

} // namespace inet

#endif

