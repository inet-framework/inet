//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __TCPTESTER_H
#define __TCPTESTER_H

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "PacketDump.h"

namespace inet {

namespace tcp {

using namespace inet::queueing;

/**
 * Base class for TCP testing modules.
 */
class INET_API TCPTesterBase : public cSimpleModule, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef outSink1;
    PassivePacketSinkRef outSink2;
    int fromASeq;
    int fromBSeq;

    PacketDump tcpdump;

  protected:
    void dump(const Ptr<const inet::tcp::TcpHeader>& seg, int payloadLength, bool fromA, const char *comment=NULL);

  public:
    TCPTesterBase();
    virtual void initialize() override;
    virtual void finish() override;

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
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
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
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
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace tcp

} // namespace inet

#endif


