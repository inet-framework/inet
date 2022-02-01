//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PCAPWRITER_H
#define __INET_PCAPWRITER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/IPcapWriter.h"
#include "inet/common/packet/recorder/PcapRecorder.h"

namespace inet {

/**
 * Dumps packets into a PCAP file; see the "pcap-savefile" man page or
 * http://www.tcpdump.org/ for details on the file format.
 * Note: The file is currently recorded in the "classic" format,
 * not in the "Next Generation" file format also on tcpdump.org.
 */
class INET_API PcapWriter : public IPcapWriter
{
  protected:
    std::string fileName;
    FILE *dumpfile = nullptr; // pcap file
    unsigned int snaplen = 0; // max. length of packets in pcap file
    PcapLinkType network = LINKTYPE_INVALID; // the network type header field in the PCAP file, see http://www.tcpdump.org/linktypes.html
    bool flush = false;
    bool needHeader = true;

  protected:
    void writeHeader(PcapLinkType linkType);

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
    void open(const char *filename, unsigned int snaplen) override;

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const override { return dumpfile != nullptr; }

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
    void setFlush(bool flush) override { this->flush = flush; }
};

} // namespace inet

#endif

