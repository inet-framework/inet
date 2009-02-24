//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#ifndef __LOCALDELIVERCORE_H__
#define __LOCALDELIVERCORE_H__

// Rewrite: Andras Varga, 2004


#include "IPFragBuf.h"
#include "ProtocolMap.h"
#include "IPDatagram.h"



/**
 * Receives IP datagram for local delivery: assemble from fragments,
 * decapsulate and forward to corresponding transport protocol.
 * More info in the NED file.
 */
class INET_API IPLocalDeliver : public cSimpleModule
{
  private:
    // fragmentation reassembly
    IPFragBuf fragbuf;
    simtime_t fragmentTimeoutTime;
    simtime_t lastCheckTime;

    // where to send different transport protocols after encapsulation
    ProtocolMapping mapping;

  private:
    cMessage *decapsulateIP(IPDatagram *);

  public:
    Module_Class_Members(IPLocalDeliver, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
