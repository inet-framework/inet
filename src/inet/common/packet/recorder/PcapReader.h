//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCAPREADER_H
#define __INET_PCAPREADER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"

namespace inet {

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

/**
 * Wall-clock capture timestamp of a pcap record, kept in the file's native
 * fields so no precision is lost and no simtime_t range limit applies. Mapping
 * this absolute timestamp onto a simulation time (e.g. relative to the first
 * record, or to a configured origin) is the caller's responsibility.
 */
struct PcapRecordTime
{
    int32_t sec = 0; /* seconds since the Unix epoch (pcap ts_sec) */
    uint32_t usec = 0; /* microseconds within the second (pcap ts_usec) */
};

class INET_API PcapReader
{
  protected:
    bool swapByteOrder = false;
    struct pcap_hdr fileHeader;
    const char *packetNameFormat = nullptr;
    FILE *file = nullptr;
    PacketPrinter packetPrinter;

  public:
    virtual void openPcap(const char *filename, const char *packetNameFormat);
    virtual void closePcap();
    virtual bool isOpen() { return file != nullptr; }
    virtual std::pair<PcapRecordTime, Packet *> readPacket();
};

} // namespace inet

#endif

