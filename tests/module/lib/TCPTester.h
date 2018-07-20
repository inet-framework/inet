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

#ifndef __TCPTESTER_H
#define __TCPTESTER_H

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/common/packet/recorder/PacketDump.h"

namespace inet {

namespace tcp {

/**
 * Base class for TCP testing modules.
 */
class INET_API TCPTesterBase : public cSimpleModule
{
  protected:
    int fromASeq;
    int fromBSeq;

    PacketDump tcpdump;

  protected:
    void dump(const Ptr<const inet::tcp::TcpHeader>& seg, int payloadLength, bool fromA, const char *comment=NULL);

  public:
    TCPTesterBase();
    virtual void initialize();
    virtual void finish();
};


/**
 * Dumps every packet using the PacketDump class, and in addition it can delete,
 * delay or duplicate TCP segments, and insert new segments.
 *
 * Script format: see NED documentation.
 */
class INET_API TCPScriptableTester : public TCPTesterBase
{
  protected:
    enum {CMD_DELETE,CMD_COPY}; // "delay" is same as "copy"
    typedef std::vector<simtime_t> DelayVector;
    struct Command
    {
        bool fromA;  // direction
        int segno;   // segment number
        int command; // CMD_DELETE, CMD_COPY
        DelayVector delays;  // arg list
    };
    typedef std::vector<Command> CommandVector;
    CommandVector commands;

  protected:
    void parseScript(const char *script);
    void dispatchSegment(Packet *pk);
    void processIncomingSegment(Packet *pk, bool fromA);

  public:
    TCPScriptableTester() {}
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};


/**
 * Randomly delete, delay etc packets.
 */
class INET_API TCPRandomTester : public TCPTesterBase
{
  protected:
    double pdelete;
    double pdelay;
    double pcopy;
    cPar *numCopies;
    cPar *delay;

  protected:
    void dispatchSegment(Packet *pk);
    void processIncomingSegment(Packet *pk, bool fromA);

  public:
    TCPRandomTester() {}
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

} // namespace tcp

} // namespace inet

#endif


