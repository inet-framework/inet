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
#include "inet/common/INETUtils.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/recorder/PcapWriter.h"

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

PcapWriter::~PcapWriter()
{
    closePcap();
}

void PcapWriter::openPcap(const char *filename, unsigned int snaplen_par, uint32 network)
{
    struct pcap_hdr fh;

    if (!filename || !filename[0])
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    inet::utils::makePathForFile(filename);
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
    fh.network = network;
    fwrite(&fh, sizeof(fh), 1, dumpfile);
}

void PcapWriter::writePacket(simtime_t stime, const Packet *packet)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    EV << "PcapWriter::writeFrame\n";
    uint8 buf[MAXBUFLENGTH];
    memset(buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;
    ph.ts_sec = (int32)stime.inUnit(SIMTIME_S);
    ph.ts_usec = (uint32)(stime.inUnit(SIMTIME_US) - (uint32)1000000 * stime.inUnit(SIMTIME_S));
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    for (size_t i = 0; i < bytes.size(); i++) {
        buf[i] = bytes[i];
    }
    ph.orig_len = B(data->getChunkLength()).get();

    ph.incl_len = ph.orig_len > snaplen ? snaplen : ph.orig_len;
    fwrite(&ph, sizeof(ph), 1, dumpfile);
    fwrite(buf, ph.incl_len, 1, dumpfile);
    if (flush)
        fflush(dumpfile);
}

void PcapWriter::closePcap()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = nullptr;
    }
}

} // namespace inet

