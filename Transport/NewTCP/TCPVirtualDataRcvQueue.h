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

#ifndef __TCPVIRTUALDATARCVQUEUE_H
#define __TCPVIRTUALDATARCVQUEUE_H

#include <omnetpp.h>
#include <list>
#include "TCPSegment_m.h"
#include "TCPReceiveQueue.h"

/**
 *
 */
class TCPVirtualDataRcvQueue : public TCPReceiveQueue
{
  protected:
    struct Region
    {
        uint32 begin;
        uint32 end;
    };
    typedef std::list<Region> RegionList;
    RegionList regionList;

  public:
    /**
     * Ctor.
     */
    TCPVirtualDataRcvQueue();

    /**
     * Virtual dtor.
     */
    virtual ~TCPVirtualDataRcvQueue();

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


