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

#ifndef __TCPDUMP_H
#define __TCPDUMP_H

#include <omnetpp.h>
#include "IPAddress.h"
#include "IPDatagram_m.h"
#include "TCPSegment_m.h"


/**
 * Dumps TCP packets in tcpdump format.
 */
class TCPDump
{
  protected:
    std::ostream *outp;
  public:
    TCPDump(std::ostream& o);
    void dump(IPDatagram *dgram);
    void dump(TCPSegment *tcpseg, IPAddress srcAddr, IPAddress destAddr);
};

/**
 *
 */
class TCPDumpModule : public cSimpleModule
{
  protected:
    TCPDump tcpdump;
  public:
    TCPDumpModule(const char *name, cModule *parent);
    virtual void handleMessage(cMessage *msg);
};

#endif


