//
// Copyright (C) 2005 Michael Tuexen
//               2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __TCPDUMP_H
#define __TCPDUMP_H

#include <omnetpp.h>
#include "IPAddress.h"
//#include "IPDatagram_m.h"
#include "IPDatagram.h"
#include "SCTPMessage.h"
#include "TCPSegment.h"
#include "IPv6Datagram_m.h"

#define PCAP_MAGIC          0xa1b2c3d4

/* "libpcap" file header (minus magic number). */
struct pcap_hdr {
    uint32  magic;      /* magic */
    uint16  version_major;  /* major version number */
    uint16  version_minor;  /* minor version number */
    uint32  thiszone;   /* GMT to local correction */
    uint32  sigfigs;    /* accuracy of timestamps */
    uint32  snaplen;    /* max length of captured packets, in octets */
    uint32  network;    /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr {
    int32   ts_sec;     /* timestamp seconds */
    uint32  ts_usec;    /* timestamp microseconds */
    uint32  incl_len;   /* number of octets of packet saved in file */
    uint32  orig_len;   /* actual length of packet */
};

typedef struct {
    uint8  dest_addr[6];
    uint8  src_addr[6];
    uint16 l3pid;
} hdr_ethernet_t;

static hdr_ethernet_t HDR_ETHERNET = {
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    0};

/**
 * Dumps SCTP packets in tcpdump format.
 */
class TCPDumper
{
    protected:
        int32 seq;
        std::ostream *outp;
    public:
        TCPDumper(std::ostream& o);
        ~TCPDumper();
        void ipDump(const char *label, IPDatagram *dgram, const char *comment=NULL);
        void sctpDump(const char *label, SCTPMessage *sctpmsg, const std::string& srcAddr, const std::string& destAddr, const char *comment=NULL);
        void dump(const char *label, const char *msg);
        void tcpDump(bool l2r, const char *label, IPDatagram *dgram, const char *comment=NULL);
        void tcpDump(bool l2r, const char *label, TCPSegment *tcpseg, const std::string& srcAddr, const std::string& destAddr, const char *comment=NULL);
        void dumpIPv6(bool l2r, const char *label, IPv6Datagram_Base *dgram, const char *comment=NULL);//FIXME: Temporary hack
        void udpDump(bool l2r, const char *label, IPDatagram *dgram, const char *comment);
        const char *intToChunk(int32 type);
        FILE *dumpfile;
};


/**
 * Dumps every packet using the TCPDumper class
 */
class TCPDump : public cSimpleModule
{
    protected:
        TCPDumper tcpdump;
    public:
        TCPDump();
        ~TCPDump();
        TCPDump(const char *name, cModule *parent);
        virtual void handleMessage(cMessage *msg);
        virtual void initialize();
        virtual void finish();
};

#endif


