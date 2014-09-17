//
// Copyright (C) 2005 Michael Tuexen
//               2008 Irene Ruengeler
//               2009 Thomas Dreibholz
//               2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_TCPDUMP_H
#define __INET_TCPDUMP_H

#include "inet/common/INETDefs.h"

#include "inet/common/packet/PcapDump.h"
#include "inet/common/packet/PacketDump.h"

namespace inet {

/**
 * Dumps every packet using the PcapDump class and the
 * PacketDump class.
 */
class INET_API TCPDump : public cSimpleModule
{
  protected:
    PcapDump pcapDump;
    PacketDump tcpdump;
    unsigned int snaplen;
    unsigned long first, last, space;
    bool dumpBadFrames;
    bool dropBadFrames;

  public:
    TCPDump();
    ~TCPDump();
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
    virtual void finish();
};

} // namespace inet

#endif // ifndef __INET_TCPDUMP_H

