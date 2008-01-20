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
#include <HeaderSerializers/headers/defs.h>

#define PCAP_MAGIC 0xa1b2c3d4

#pragma pack (1)
/* "libpcap" file header (minus magic number). */
struct pcap_hdr {
    uint32_t magic;         /* magic */
    uint16_t version_major; /* major version number */
    uint16_t version_minor; /* minor version number */
    uint32_t thiszone;      /* GMT to local correction */
    uint32_t sigfigs;       /* accuracy of timestamps */
    uint32_t snaplen;       /* max length of captured packets, in octets */
    uint32_t network;       /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr {
    uint32_t ts_sec;   /* timestamp seconds */
    uint32_t ts_usec;  /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */
};

struct eth_hdr{
    uint8_t dest_addr[6];
    uint8_t src_addr[6];
    uint16_t l3pid;
};

static struct eth_hdr dummy_ethernet_hdr = {
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    0};
#pragma pack ()

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
    FILE *dumpfile;
    unsigned char *buffer;
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
    virtual void initialize();
    virtual void finish();
};

#endif


