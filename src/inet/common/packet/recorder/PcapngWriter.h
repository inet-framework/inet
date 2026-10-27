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
 * The block types that PcapngWriter writes, see the PCAPng specification
 * (draft-ietf-opsawg-pcapng).
 */
enum PcapngBlockType : uint32_t {
    PCAPNG_INTERFACE_DESCRIPTION_BLOCK = 0x00000001,
    PCAPNG_ENHANCED_PACKET_BLOCK       = 0x00000006,
    PCAPNG_SECTION_HEADER_BLOCK        = 0x0A0D0D0A,
};

/**
 * The option codes that all block types share.
 */
enum PcapngOptionCode : uint16_t {
    PCAPNG_OPT_ENDOFOPT            = 0,
    PCAPNG_OPT_COMMENT             = 1,
    PCAPNG_OPT_CUSTOM_STRING       = 2988,
    PCAPNG_OPT_CUSTOM_BINARY       = 2989,
    PCAPNG_OPT_CUSTOM_STRING_LOCAL = 19372,
    PCAPNG_OPT_CUSTOM_BINARY_LOCAL = 19373,
};

/**
 * The option codes of the section header block.
 */
enum PcapngSectionOptionCode : uint16_t {
    PCAPNG_SHB_HARDWARE = 2,
    PCAPNG_SHB_OS       = 3,
    PCAPNG_SHB_USERAPPL = 4,
};

/**
 * The option codes of the interface description block.
 */
enum PcapngInterfaceOptionCode : uint16_t {
    PCAPNG_IF_NAME        = 2,
    PCAPNG_IF_DESCRIPTION = 3,
    PCAPNG_IF_IPV4ADDR    = 4,
    PCAPNG_IF_IPV6ADDR    = 5,
    PCAPNG_IF_MACADDR     = 6,
    PCAPNG_IF_EUIADDR     = 7,
    PCAPNG_IF_SPEED       = 8,
    PCAPNG_IF_TSRESOL     = 9,
    PCAPNG_IF_TZONE       = 10,
    PCAPNG_IF_FILTER      = 11,
    PCAPNG_IF_OS          = 12,
    PCAPNG_IF_FCSLEN      = 13,
    PCAPNG_IF_TSOFFSET    = 14,
    PCAPNG_IF_HARDWARE    = 15,
    PCAPNG_IF_TXSPEED     = 16,
    PCAPNG_IF_RXSPEED     = 17,
};

/**
 * The option codes of the enhanced packet block.
 */
enum PcapngPacketOptionCode : uint16_t {
    PCAPNG_EPB_FLAGS     = 2,
    PCAPNG_EPB_HASH      = 3,
    PCAPNG_EPB_DROPCOUNT = 4,
    PCAPNG_EPB_PACKETID  = 5,
    PCAPNG_EPB_QUEUE     = 6,
    PCAPNG_EPB_VERDICT   = 7,
};

/**
 * The reception type in bits 2-4 of the epb_flags option.
 */
enum PcapngReceptionType : uint8_t {
    PCAPNG_RECEPTION_NOT_SPECIFIED = 0,
    PCAPNG_RECEPTION_UNICAST       = 1,
    PCAPNG_RECEPTION_MULTICAST     = 2,
    PCAPNG_RECEPTION_BROADCAST     = 3,
    PCAPNG_RECEPTION_PROMISCUOUS   = 4,
};

/**
 * The link-layer errors in bits 16-31 of the epb_flags option.
 */
enum PcapngLinkLayerError : uint32_t {
    PCAPNG_ERROR_CRC                   = 1u << 24,
    PCAPNG_ERROR_PACKET_TOO_LONG       = 1u << 25,
    PCAPNG_ERROR_PACKET_TOO_SHORT      = 1u << 26,
    PCAPNG_ERROR_INTERFRAME_GAP        = 1u << 27,
    PCAPNG_ERROR_UNALIGNED_FRAME       = 1u << 28,
    PCAPNG_ERROR_START_FRAME_DELIMITER = 1u << 29,
    PCAPNG_ERROR_PREAMBLE              = 1u << 30,
    PCAPNG_ERROR_SYMBOL                = 1u << 31,
};

/**
 * The value of the epb_flags option. A zero field means that the information
 * is not available. Other bits of the option need PcapngOptions::addUint32().
 */
struct INET_API PcapngPacketFlags
{
    Direction direction = DIRECTION_UNDEFINED;
    PcapngReceptionType receptionType = PCAPNG_RECEPTION_NOT_SPECIFIED;
    B fcsLength = B(0); // at most 15 bytes, it overrides the if_fcslen option of the interface
    uint32_t linkLayerErrors = 0; // the sum of PcapngLinkLayerError values

    uint32_t getValue() const;
};

/**
 * The options of one PCAPng block, in the order of writing. The writer adds
 * the padding of each value and the end-of-options marker. Numbers use the
 * byte order of the host, which the section header block declares. The
 * subclasses add the options of one block type, because the option codes of
 * different block types overlap.
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

    /**
     * Adds an option with any code, for example a custom option. A custom
     * option value starts with a Private Enterprise Number in host byte order.
     */
    void addOption(uint16_t code, const std::vector<uint8_t>& value);
    void addString(uint16_t code, const std::string& value);
    void addUint8(uint16_t code, uint8_t value);
    void addUint32(uint16_t code, uint32_t value);
    void addUint64(uint16_t code, uint64_t value);

    /**
     * Adds an opt_comment option. A block can have more than one.
     */
    void addComment(const std::string& comment) { addString(PCAPNG_OPT_COMMENT, comment); }
};

/**
 * The options of a section header block.
 */
class INET_API PcapngSectionOptions : public PcapngOptions
{
};

/**
 * The options of an interface description block. The writer adds the
 * if_tsresol option from the time precision of the interface.
 */
class INET_API PcapngInterfaceOptions : public PcapngOptions
{
  public:
    void addName(const std::string& name) { addString(PCAPNG_IF_NAME, name); }
    void addDescription(const std::string& description) { addString(PCAPNG_IF_DESCRIPTION, description); }
    void addIpv4Address(const Ipv4Address& address, const Ipv4Address& netmask);
    void addMacAddress(const MacAddress& address);
    void addFcsLength(b fcsLength);
};

/**
 * The options of an enhanced packet block.
 */
class INET_API PcapngPacketOptions : public PcapngOptions
{
  public:
    void addFlags(const PcapngPacketFlags& flags) { addUint32(PCAPNG_EPB_FLAGS, flags.getValue()); }
};

/**
 * Dumps packets into a PCAP Next Generation file; see the PCAPng specification
 * (draft-ietf-opsawg-pcapng) for details on the file format.
 *
 * The block functions write the blocks of the format directly. The functions
 * of IPcapWriter map INET objects to these blocks: open() writes a section
 * header block, and writePacket() writes an interface description block for
 * each pair of network interface and link type before its first packet. A
 * caller can use both kinds of functions for one file.
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
    bool hasSection = false;
    std::vector<Interface> interfaces; // the interfaces of the current section, the index is the interface ID
    std::map<std::pair<int, PcapLinkType>, int> interfaceModuleIdAndLinkTypeToPcapngInterfaceId;
    std::map<PcapLinkType, int> linkTypeToPcapngInterfaceIdWithoutNetworkInterface;

  protected:
    void writeBlock(uint32_t blockType, const std::vector<uint8_t>& fields, const uint8_t *data, size_t dataLength, const PcapngOptions& options);

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
     * Opens a PCAP file with the given file name, and writes a section header
     * block. The snaplen and the time precision apply to the interfaces of
     * writePacket(). Throws an exception if the file cannot be opened.
     */
    void open(const char *filename, unsigned int snaplen, int timePrecision) override;

    /**
     * Opens a PCAP file with the given file name, and writes nothing. The
     * first block must be a section header block. Throws an exception if the
     * file cannot be opened.
     */
    void open(const char *filename);

    /**
     * Returns true if the pcap file is currently open.
     */
    bool isOpen() const override { return dumpfile != nullptr; }

    /**
     * Writes a section header block, which starts a new section. The interface
     * IDs of the previous section become invalid.
     */
    void writeSectionHeaderBlock(const PcapngSectionOptions& options = PcapngSectionOptions());

    /**
     * Writes an interface description block, and returns its interface ID. A
     * zero snaplen means no limit. The time precision is the negative decimal
     * exponent of the timestamp unit, for example 6 for microseconds.
     */
    int writeInterfaceDescriptionBlock(PcapLinkType linkType, unsigned int snaplen, int timePrecision, const PcapngInterfaceOptions& options = PcapngInterfaceOptions());

    /**
     * Writes an enhanced packet block. The block keeps at most snaplen bytes
     * of the data. The original length is the length of the packet before a
     * truncation, so it cannot be less than the data length.
     */
    void writeEnhancedPacketBlock(int interfaceId, simtime_t time, const uint8_t *data, size_t dataLength, size_t originalLength, const PcapngPacketOptions& options = PcapngPacketOptions());

    /**
     * Records the interface into the output file, and returns its interface ID.
     * Without a network interface, the interface description has no name and no
     * addresses.
     */
    int writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType);

    /**
     * Records the given packet into the output file if it is open,
     * and throws an exception otherwise. An undefined direction leaves
     * the direction of the packet unspecified.
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

