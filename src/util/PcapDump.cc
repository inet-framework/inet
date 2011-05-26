//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
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


#include <errno.h>

#include "PcapDump.h"

#include "IPProtocolId_m.h"

#ifdef WITH_UDP
#include "UDPPacket_m.h"
#endif

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#include "IPv4Serializer.h"
#endif


#define MAXBUFLENGTH 65536

#define PCAP_MAGIC           0xa1b2c3d4

/* "libpcap" file header (minus magic number). */
struct pcap_hdr {
     uint32 magic;      /* magic */
     uint16 version_major;   /* major version number */
     uint16 version_minor;   /* minor version number */
     uint32 thiszone;   /* GMT to local correction */
     uint32 sigfigs;        /* accuracy of timestamps */
     uint32 snaplen;        /* max length of captured packets, in octets */
     uint32 network;        /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr {
     int32  ts_sec;     /* timestamp seconds */
     uint32 ts_usec;        /* timestamp microseconds */
     uint32 incl_len;   /* number of octets of packet saved in file */
     uint32 orig_len;   /* actual length of packet */
};



PcapDump::PcapDump()
{
     dumpfile = NULL;
}

PcapDump::~PcapDump()
{
    closePcap();
}

void PcapDump::openPcap(const char* filename, unsigned int snaplen_par)
{
    struct pcap_hdr fh;

    if (!filename || !filename[0])
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    dumpfile = fopen(filename, "wb");

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    snaplen = snaplen_par;

    fh.magic = PCAP_MAGIC;
    fh.version_major = 2;
    fh.version_minor = 4;
    fh.thiszone = 0;
    fh.sigfigs = 0;
    fh.snaplen = snaplen;
    fh.network = 0;
    fwrite(&fh, sizeof(fh), 1, dumpfile);
}

void PcapDump::writeFrame(simtime_t stime, const IPv4Datagram *ipPacket)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

#ifdef WITH_IPv4
    uint8 buf[MAXBUFLENGTH];
    memset((void*)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;
    ph.ts_sec = (int32)stime.dbl();
    ph.ts_usec = (uint32)((stime.dbl() - ph.ts_sec) * 1000000);
     // Write Ethernet header
    uint32 hdr = 2; //AF_INET

    int32 serialized_ip = IPv4Serializer().serialize(ipPacket, buf, sizeof(buf), true);
    ph.orig_len = serialized_ip + sizeof(uint32);

    ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
    fwrite(&ph, sizeof(ph), 1, dumpfile);
    fwrite(&hdr, sizeof(uint32), 1, dumpfile);
    fwrite(buf, ph.incl_len - sizeof(uint32), 1, dumpfile);
#else
    throw cRuntimeError("Cannot write frame: INET compiled without IPv4 feature");
#endif
}

void PcapDump::closePcap()
{
    if (dumpfile)
    {
        fclose(dumpfile);
        dumpfile = NULL;
    }
}

