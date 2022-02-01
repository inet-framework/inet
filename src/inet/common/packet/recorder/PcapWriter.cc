//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/packet/recorder/PcapWriter.h"

#include <cerrno>

#include "inet/common/INETUtils.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

#define MAXBUFLENGTH    65536

#define PCAP_MAGIC      0xa1b2c3d4

/* "libpcap" file header (minus magic number). */
struct pcap_hdr
{
    uint32_t magic; /* magic */
    uint16_t version_major; /* major version number */
    uint16_t version_minor; /* minor version number */
    uint32_t thiszone; /* GMT to local correction */
    uint32_t sigfigs; /* accuracy of timestamps */
    uint32_t snaplen; /* max length of captured packets, in octets */
    uint32_t network; /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr
{
    int32_t ts_sec; /* timestamp seconds */
    uint32_t ts_usec; /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */
};

PcapWriter::~PcapWriter()
{
    PcapWriter::close(); // NOTE: admitting that this will not call overridden methods from the destructor
}

void PcapWriter::open(const char *filename, unsigned int snaplen_par)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    inet::utils::makePathForFile(filename);
    dumpfile = fopen(filename, "wb");
    fileName = filename;

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    snaplen = snaplen_par;
    needHeader = true;
    network = LINKTYPE_INVALID;

    flush = false;
}

void PcapWriter::writeHeader(PcapLinkType linkType)
{
    struct pcap_hdr fh;

    fh.magic = PCAP_MAGIC;
    fh.version_major = 2;
    fh.version_minor = 4;
    fh.thiszone = 0;
    fh.sigfigs = 0;
    fh.snaplen = snaplen;
    fh.network = linkType;
    fwrite(&fh, sizeof(fh), 1, dumpfile);
}

void PcapWriter::writePacket(simtime_t stime, const Packet *packet, Direction direction, NetworkInterface *ie, PcapLinkType linkTypePar)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    if (needHeader) {
        if (linkTypePar == LINKTYPE_INVALID)
            throw cRuntimeError("invalid linktype arrived");
        writeHeader(linkTypePar);
        network = linkTypePar;
        needHeader = false;
    }
    else {
        if (network != linkTypePar)
            throw cRuntimeError("linktype mismatch error: required linktype = %d, arrived linktype = %d", network, linkTypePar);
    }

    (void)direction; // unused
    (void)ie; // unused

    EV_INFO << "Writing packet" << EV_FIELD(packet) << EV_FIELD(fileName) << EV_ENDL;
    uint8_t buf[MAXBUFLENGTH];
    memset(buf, 0, sizeof(buf));

    struct pcaprec_hdr ph;
    ph.ts_sec = (int32_t)stime.inUnit(SIMTIME_S);
    ph.ts_usec = (uint32_t)(stime.inUnit(SIMTIME_US) - (uint32_t)1000000 * stime.inUnit(SIMTIME_S));
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

void PcapWriter::close()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = nullptr;
    }
}

} // namespace inet

