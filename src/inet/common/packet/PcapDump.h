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

#ifndef __INET_PCAPDUMP_H
#define __INET_PCAPDUMP_H

#include "inet/common/INETDefs.h"

namespace inet {

// Foreign declarations:
class IPv4Datagram;
class IPv6Datagram;

/**
 * Dumps packets into a PCAP file; see the "pcap-savefile" man page or
 * http://www.tcpdump.org/ for details on the file format.
 * Note: The file is currently recorded in the "classic" format,
 * not in the "Next Generation" file format also on tcpdump.org.
 */
class PcapDump
{
  protected:
    FILE *dumpfile;    // pcap file
    unsigned int snaplen;    // max. length of packets in pcap file

  public:
    /**
     * Constructor. It does not open the output file.
     */
    PcapDump();

    /**
     * Destructor. It closes the output file if it is open.
     */
    ~PcapDump();

    /**
     * Opens a PCAP file with the given file name. The snaplen parameter
     * is the length that packets will be truncated to. Throws an exception
     * if the file cannot be opened.
     */
    void openPcap(const char *filename, unsigned int snaplen);

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const { return dumpfile != NULL; }

    /**
     * Records the given packet into the output file if it is open,
     * and throws an exception otherwise.
     */
    void writeFrame(simtime_t time, const IPv4Datagram *ipPacket);
    void writeIPv6Frame(simtime_t stime, const IPv6Datagram *ipPacket);

    /**
     * Closes the output file if it is open.
     */
    void closePcap();
};

} // namespace inet

#endif // ifndef __INET_PCAPDUMP_H

