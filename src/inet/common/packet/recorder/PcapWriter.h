//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
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

#ifndef __INET_PCAPWRITER_H
#define __INET_PCAPWRITER_H

#include "inet/common/packet/Packet.h"

namespace inet {

// Foreign declarations:
class Ipv4Header;
class Ipv6Header;

/**
 * Dumps packets into a PCAP file; see the "pcap-savefile" man page or
 * http://www.tcpdump.org/ for details on the file format.
 * Note: The file is currently recorded in the "classic" format,
 * not in the "Next Generation" file format also on tcpdump.org.
 */
class INET_API PcapWriter
{
  protected:
    FILE *dumpfile = nullptr;    // pcap file
    unsigned int snaplen = 0;    // max. length of packets in pcap file
    bool flush = false;

  public:
    /**
     * Constructor. It does not open the output file.
     */
    PcapWriter() {}

    /**
     * Destructor. It closes the output file if it is open.
     */
    ~PcapWriter();

    /**
     * Opens a PCAP file with the given file name. The snaplen parameter
     * is the length that packets will be truncated to. Throws an exception
     * if the file cannot be opened.
     */
    void openPcap(const char *filename, unsigned int snaplen, uint32 network);

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const { return dumpfile != nullptr; }

    /**
     * Records the given packet into the output file if it is open,
     * and throws an exception otherwise.
     */
    void writePacket(simtime_t time, const Packet *packet);

    /**
     * Closes the output file if it is open.
     */
    void closePcap();

    /**
     * Force flushing of pcap dump.
     */
    void setFlushParameter(bool doFlush) { flush = doFlush; };
};

} // namespace inet

#endif // ifndef __INET_PCAPWRITER_H

