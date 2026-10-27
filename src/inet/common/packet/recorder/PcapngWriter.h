//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PCAPNGWRITER_H
#define __INET_PCAPNGWRITER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/IPcapWriter.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

/**
 * The options of one PCAPng block, in the order of writing. The writer adds
 * the padding of each value and the end-of-options marker. Numbers use the
 * byte order of the host, which the section header block declares.
 */
class INET_API PcapngOptions
{
  public:
    struct Option {
        uint16_t code;
        std::vector<uint8_t> value;
    };

  protected:
    std::vector<Option> options;

  public:
    bool isEmpty() const { return options.empty(); }
    const std::vector<Option>& getOptions() const { return options; }

    void addOption(uint16_t code, const std::vector<uint8_t>& value);
    void addString(uint16_t code, const std::string& value);
    void addUint8(uint16_t code, uint8_t value);
    void addUint32(uint16_t code, uint32_t value);
};

/**
 * Dumps packets into a PCAP Next Generation file; see the "pcap-savefile"
 * man page or http://www.tcpdump.org/ for details on the file format.
 */
class INET_API PcapngWriter : public IPcapWriter
{
  protected:
    struct Interface {
        unsigned int snaplen;
        int timePrecision;
    };

    std::string fileName;
    FILE *dumpfile = nullptr; // pcap file
    unsigned int snaplen = 0;
    bool flush = false;
    int timePrecision = 6;
    std::vector<Interface> interfaces; // the interfaces of the current section, the index is the interface ID
    std::map<std::pair<int, PcapLinkType>, int> interfaceModuleIdAndLinkTypeToPcapngInterfaceId;
    std::map<PcapLinkType, int> linkTypeToPcapngInterfaceIdWithoutNetworkInterface;

  protected:
    void writeBlock(uint32_t blockType, const std::vector<uint8_t>& fields, const uint8_t *data, size_t dataLength, const PcapngOptions& options);
    void writeSectionHeaderBlock(const PcapngOptions& options);
    int writeInterfaceDescriptionBlock(PcapLinkType linkType, unsigned int snaplen, int timePrecision, const PcapngOptions& options);
    void writeEnhancedPacketBlock(int interfaceId, simtime_t time, const uint8_t *data, size_t dataLength, size_t originalLength, const PcapngOptions& options);

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
    void open(const char *filename, unsigned int snaplen, int timePrecision) override;

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const override { return dumpfile != nullptr; }

    /**
     * Records the interface into the output file, and returns its interface ID.
     * Without a network interface, the interface description has no name and no
     * addresses.
     */
    int writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType);

    /**
     * Records the given packet into the output file if it is open,
     * and throws an exception otherwise.
     */
    void writePacket(simtime_t time, const Packet *packet, b frontOffset, b backOffset, Direction direction, NetworkInterface *ie, PcapLinkType linkType) override;

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

