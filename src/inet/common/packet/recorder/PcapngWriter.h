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

#ifndef __INET_PCAPNGWRITER_H
#define __INET_PCAPNGWRITER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/IPcapWriter.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

/**
 * Dumps packets into a PCAP Next Generation file; see the "pcap-savefile"
 * man page or http://www.tcpdump.org/ for details on the file format.
 */
class INET_API PcapngWriter : public IPcapWriter
{
  protected:
    std::string fileName;
    FILE *dumpfile = nullptr;    // pcap file
    bool flush = false;
    int nextPcapngInterfaceId = 0;
    std::map<int, int> interfaceModuleIdToPcapngInterfaceId;

  public:
    /**
     * Constructor. It does not open the output file.
     */
    PcapngWriter() {}

    /**
     * Destructor. It closes the output file if it is open.
     */
    ~PcapngWriter();

    /**
     * Opens a PCAP file with the given file name. Throws an exception
     * if the file cannot be opened.
     */
    void open(const char *filename, unsigned int snaplen) override;

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const override { return dumpfile != nullptr; }

    /**
     * Records the interface into the output file.
     */
    void writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType);

    /**
     * Records the given packet into the output file if it is open,
     * and throws an exception otherwise.
     */
    void writePacket(simtime_t time, const Packet *packet, Direction direction, NetworkInterface *ie, PcapLinkType linkType) override;

    /**
     * Closes the output file if it is open.
     */
    void close() override;

    /**
     * Force flushing of pcap dump.
     */
    void setFlush(bool flush) override { this->flush = flush; };
};

} // namespace inet

#endif

