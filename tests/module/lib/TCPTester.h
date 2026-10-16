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
#include "PacketDump.h"

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
    int segmentEcn = -1; // the IP ECN codepoint of the segment that dump() shows

  protected:
    void dump(const Ptr<const inet::tcp::TcpHeader>& seg, int payloadLength, bool fromA, const char *comment=NULL);
    // the tester has no IP layer: it delivers the ECN codepoint that the sender requested, as the network does, and returns it
    int deliverEcn(Packet *pk);

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
    enum {CMD_DELETE,CMD_COPY,CMD_CE,CMD_TSECR,CMD_DATA,CMD_NODATA,CMD_STRIP,CMD_ACE,CMD_ZEROACCECN,CMD_ACKADD,CMD_ICMP,CMD_ICMP6,CMD_COOKIE}; // CMD_COOKIE cuts the cookie of the Fast Open option to a length, for a peer that sends a part of its cookie; // CMD_NODATA removes the payload of a segment, for a peer that drops the data of a SYN (Linux without a valid Fast Open cookie); CMD_TSECR sets the TSecr of the timestamp option, for tests of the TSecr checks; CMD_DATA adds bytes and the FIN bit to a segment, for tests of a peer that sends them where INET does not (on a SYN-ACK); CMD_STRIP removes the options of a kind, for a peer that does not answer an option; CMD_ACE sets the AE, CWR and ECE bits, for a peer or a path that changes them; CMD_ZEROACCECN sets the fields of the AccECN option to 0, for a path that zeroes them; // "delay" is same as "copy"; CMD_CE marks the segment CE, ECT(1) or ECT(0) instead of forwarding it unmarked -- for AccECN/ECN module tests that need a CE-marked inbound segment
    typedef std::vector<simtime_t> DelayVector;
    struct Command
    {
        bool fromA;  // direction
        int segno;   // segment number
        int command; // CMD_DELETE, CMD_COPY
        DelayVector delays;  // arg list
        uint32_t tsecr = 0;  // CMD_TSECR: the new TSecr
        uint32_t ackadd = 0; // CMD_ACKADD: the number to add to the ACK field, for an ACK of data that was not sent
        int icmpType = 0;    // CMD_ICMP, CMD_ICMP6: the type and the code of the ICMP error that answers the segment in place of the peer
        int icmpCode = 0;
        uint32_t icmpSeqAdd = 0; // CMD_ICMP, CMD_ICMP6: the number to add to the quoted sequence number, for a stale error
        int icmpMtu = 0; // CMD_ICMP, CMD_ICMP6: the MTU of a fragmentation-needed error or a Packet Too Big
        int bytes = 0;       // CMD_DATA: the number of bytes to add after the payload
        bool fin = false;    // CMD_DATA: set the FIN bit
        int kind = -1;       // CMD_STRIP: the kind of the options to remove
        int cookieLength = 0; // CMD_COOKIE: the new length of the cookie of the Fast Open option
        int ace = 0;         // CMD_ACE: the new (AE,CWR,ECE) bits, as a number from 0 to 7
        int ecn = 3;         // CMD_CE: the new IP ECN codepoint (IP_ECN_CE, IP_ECN_ECT_1 or IP_ECN_ECT_0)
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


