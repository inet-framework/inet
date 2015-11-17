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

#include "inet/common/packet/PcapDump.h"

#include "inet/common/serializer/SerializerBase.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDPPacket_m.h"
#endif // ifdef WITH_UDP

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/common/serializer/ipv6/IPv6Serializer.h"
#endif // ifdef WITH_IPv6

namespace inet {

#define MAXBUFLENGTH    65536

#define PCAP_MAGIC      0xa1b2c3d4

/* "libpcap" file header (minus magic number). */
struct pcap_hdr
{
    uint32 magic;    /* magic */
    uint16 version_major;    /* major version number */
    uint16 version_minor;    /* minor version number */
    uint32 thiszone;    /* GMT to local correction */
    uint32 sigfigs;    /* accuracy of timestamps */
    uint32 snaplen;    /* max length of captured packets, in octets */
    uint32 network;    /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr
{
    int32 ts_sec;    /* timestamp seconds */
    uint32 ts_usec;    /* timestamp microseconds */
    uint32 incl_len;    /* number of octets of packet saved in file */
    uint32 orig_len;    /* actual length of packet */
};

PcapDump::~PcapDump()
{
    closePcap();
}

void PcapDump::openPcap(const char *filename, unsigned int snaplen_par)
{
    struct pcap_hdr fh;

    if (!filename || !filename[0])
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    dumpfile = fopen(filename, "wb");

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    snaplen = snaplen_par;

    flush = false;

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
    memset((void *)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;
    ph.ts_sec = (int32)stime.inUnit(SIMTIME_S);
    ph.ts_usec = (uint32)(stime.inUnit(SIMTIME_US) - (uint32)1000000 * stime.inUnit(SIMTIME_S));
    // Write Ethernet header
    uint32 hdr = 2;    //AF_INET

    serializer::Buffer b(buf, sizeof(buf));
    serializer::Context c;
    c.throwOnSerializerNotFound = false;
    serializer::IPv4Serializer().serializePacket(ipPacket, b, c);
    int32 serialized_ip = b.getPos();

    ph.orig_len = serialized_ip + sizeof(uint32);

    ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
    fwrite(&ph, sizeof(ph), 1, dumpfile);
    fwrite(&hdr, sizeof(uint32), 1, dumpfile);
    fwrite(buf, ph.incl_len - sizeof(uint32), 1, dumpfile);
    if (flush)
        fflush(dumpfile);
#else // ifdef WITH_IPv4
    throw cRuntimeError("Cannot write frame: INET compiled without IPv4 feature");
#endif // ifdef WITH_IPv4
}

void PcapDump::writeIPv6Frame(simtime_t stime, const IPv6Datagram *ipPacket)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

#ifdef WITH_IPv6
    uint8 buf[MAXBUFLENGTH];
    memset((void *)&buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;
    ph.ts_sec = (int32)stime.inUnit(SIMTIME_S);
    ph.ts_usec = (uint32)(stime.inUnit(SIMTIME_US) - (uint32)1000000 * stime.inUnit(SIMTIME_S));
    // Write Ethernet header
    uint32 hdr = 2;    //AF_INET

    serializer::Buffer b(buf, sizeof(buf));
    serializer::Context c;
    c.throwOnSerializerNotFound = false;
    serializer::IPv6Serializer().serializePacket(ipPacket, b, c);
    int32 serialized_ip = b.getPos();

    if (serialized_ip > 0) {
        ph.orig_len = serialized_ip + sizeof(uint32);

        ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
        fwrite(&ph, sizeof(ph), 1, dumpfile);
        fwrite(&hdr, sizeof(uint32), 1, dumpfile);
        fwrite(buf, ph.incl_len - sizeof(uint32), 1, dumpfile);
        if (flush)
            fflush(dumpfile);
    }
#else // ifdef WITH_IPv6
    throw cRuntimeError("Cannot write frame: INET compiled without IPv6 feature");
#endif // ifdef WITH_IPv6
}

void PcapDump::closePcap()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = nullptr;
    }
}

} // namespace inet

