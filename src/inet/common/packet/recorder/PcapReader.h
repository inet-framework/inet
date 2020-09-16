//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_PCAPREADER_H
#define __INET_PCAPREADER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"

namespace inet {

/* "libpcap" file header (minus magic number). */
struct pcap_hdr
{
    uint32_t magic;    /* magic */
    uint16_t version_major;    /* major version number */
    uint16_t version_minor;    /* minor version number */
    uint32_t thiszone;    /* GMT to local correction */
    uint32_t sigfigs;    /* accuracy of timestamps */
    uint32_t snaplen;    /* max length of captured packets, in octets */
    uint32_t network;    /* data link type */
};

/* "libpcap" record header. */
struct pcaprec_hdr
{
    int32_t ts_sec;    /* timestamp seconds */
    uint32_t ts_usec;    /* timestamp microseconds */
    uint32_t incl_len;    /* number of octets of packet saved in file */
    uint32_t orig_len;    /* actual length of packet */
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
    virtual std::pair<simtime_t, Packet *> readPacket();
};

} // namespace inet

#endif

