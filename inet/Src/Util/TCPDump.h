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
#include "IPvXAddress.h"
#include "IPDatagram_m.h"
#include "IPv6Datagram_m.h"
#include "TCPSegment.h"


/**
 * Dumps TCP packets in tcpdump format.
 */
class INET_API TCPDumper
{
  protected:
    int seq;
    std::ostream *outp;
  public:
    TCPDumper(std::ostream& o);
    void dump(bool l2r, const char *label, IPDatagram *dgram, const char *comment=NULL);
    void dumpIPv6(bool l2r, const char *label, IPv6Datagram_Base *dgram, const char *comment=NULL);//FIXME: Temporary hack
    void dump(bool l2r, const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment=NULL);
    // dumps arbitary text
    void dump(const char *label, const char *msg);
};


/**
 * Dumps every packet using the TCPDumper class
 */
class INET_API TCPDump : public cSimpleModule
{
  protected:
    TCPDumper tcpdump;
  public:
    TCPDump(const char *name=NULL, cModule *parent=NULL); // TODO remove args for later omnetpp versions
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


