//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_PCAPREADER_H
#define __INET_PCAPREADER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/printer/PacketPrinter.h"

namespace inet {

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

#endif // ifndef __INET_PCAPREADER_H

