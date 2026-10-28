//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/recorder/PcapngWriter.h"

#include <cerrno>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet {

#define PCAP_MAGIC    0x1a2b3c4d

static int pad(int value, int multiplier = 4)
{
    return (multiplier - value % multiplier) % multiplier;
}

static int roundUp(int value, int multiplier = 4)
{
    return value + pad(value, multiplier);
}

template<typename T>
static void appendValue(std::vector<uint8_t>& bytes, T value)
{
    auto valueBytes = reinterpret_cast<const uint8_t *>(&value);
    bytes.insert(bytes.end(), valueBytes, valueBytes + sizeof(value));
}

uint32_t PcapngPacketFlags::getValue() const
{
    uint32_t value = 0;
    switch (direction) {
        case DIRECTION_UNDEFINED:
            break;
        case DIRECTION_INBOUND:
            value |= 0b01;
            break;
        case DIRECTION_OUTBOUND:
            value |= 0b10;
            break;
        default:
            throw cRuntimeError("Unknown direction value");
    }
    if (receptionType > PCAPNG_RECEPTION_PROMISCUOUS)
        throw cRuntimeError("Unknown PCAPng reception type: %d", receptionType);
    value |= receptionType << 2;
    if (fcsLength < B(0) || fcsLength > B(15))
        throw cRuntimeError("The PCAPng FCS length of a packet must be between 0 and 15 bytes: %s", fcsLength.str().c_str());
    value |= fcsLength.get<B>() << 5;
    if ((linkLayerErrors & 0x0000FFFF) != 0)
        throw cRuntimeError("The PCAPng link-layer errors use bits 16-31 only: 0x%08x", linkLayerErrors);
    value |= linkLayerErrors;
    return value;
}

void PcapngOptions::addOption(uint16_t code, const std::vector<uint8_t>& value)
{
    if (value.size() > UINT16_MAX)
        throw cRuntimeError("The value of PCAPng option %d is longer than %d bytes", code, UINT16_MAX);
    options.push_back({code, value});
}

void PcapngOptions::addString(uint16_t code, const std::string& value)
{
    addOption(code, std::vector<uint8_t>(value.begin(), value.end()));
}

void PcapngOptions::addUint8(uint16_t code, uint8_t value)
{
    addOption(code, {value});
}

void PcapngOptions::addUint32(uint16_t code, uint32_t value)
{
    std::vector<uint8_t> bytes;
    appendValue(bytes, value);
    addOption(code, bytes);
}

void PcapngOptions::addUint64(uint16_t code, uint64_t value)
{
    std::vector<uint8_t> bytes;
    appendValue(bytes, value);
    addOption(code, bytes);
}

void PcapngInterfaceOptions::addIpv4Address(const Ipv4Address& address, const Ipv4Address& netmask)
{
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 4; i++) bytes.push_back(address.getDByte(i));
    for (int i = 0; i < 4; i++) bytes.push_back(netmask.getDByte(i));
    addOption(PCAPNG_IF_IPV4ADDR, bytes);
}

void PcapngInterfaceOptions::addMacAddress(const MacAddress& address)
{
    uint8_t bytes[6];
    address.getAddressBytes(bytes);
    addOption(PCAPNG_IF_MACADDR, std::vector<uint8_t>(bytes, bytes + 6));
}

void PcapngInterfaceOptions::addFcsLength(b fcsLength)
{
    if (fcsLength < b(0) || fcsLength > b(UINT8_MAX))
        throw cRuntimeError("The PCAPng FCS length of an interface must be between 0 and %d bits: %s", UINT8_MAX, fcsLength.str().c_str());
    addUint8(PCAPNG_IF_FCSLEN, fcsLength.get<b>());
}

PcapngWriter::~PcapngWriter()
{
    PcapngWriter::close(); // NOTE: admitting that this will not call overridden methods from the destructor
}

void PcapngWriter::open(const char *filename, unsigned int snaplen, int timePrecision)
{
    open(filename);
    this->snaplen = snaplen;

    // TODO check validity of timePrecision
    this->timePrecision = timePrecision;

    writeSectionHeaderBlock();
}

void PcapngWriter::open(const char *filename)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    inet::utils::makePathForFile(filename);
    dumpfile = fopen(filename, "wb");
    fileName = filename;

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    flush = false;
    hasSection = false;
    interfaces.clear();
    interfaceModuleIdAndLinkTypeToPcapngInterfaceId.clear();
    linkTypeToPcapngInterfaceIdWithoutNetworkInterface.clear();
}

void PcapngWriter::writeBlock(uint32_t blockType, const std::vector<uint8_t>& fields, const uint8_t *data, size_t dataLength, const PcapngOptions& options)
{
    if (!dumpfile)
        throw cRuntimeError("Cannot write block: pcap output file is not open");

    // options
    std::vector<uint8_t> optionBytes;
    if (!options.isEmpty()) {
        for (const auto& option : options.getOptions()) {
            appendValue<uint16_t>(optionBytes, option.code);
            appendValue<uint16_t>(optionBytes, option.value.size());
            optionBytes.insert(optionBytes.end(), option.value.begin(), option.value.end());
            optionBytes.resize(roundUp(optionBytes.size()));
        }
        // end of options
        optionBytes.resize(optionBytes.size() + 4);
    }

    ASSERT(fields.size() % 4 == 0);
    uint32_t blockTotalLength = 4 + 4 + fields.size() + roundUp(dataLength) + optionBytes.size() + 4;

    // header
    fwrite(&blockType, sizeof(blockType), 1, dumpfile);
    fwrite(&blockTotalLength, sizeof(blockTotalLength), 1, dumpfile);
    fwrite(fields.data(), fields.size(), 1, dumpfile);

    if (dataLength != 0) {
        // data
        fwrite(data, dataLength, 1, dumpfile);

        // data padding
        char padding[] = { 0, 0, 0, 0 };
        fwrite(padding, pad(dataLength), 1, dumpfile);
    }

    fwrite(optionBytes.data(), optionBytes.size(), 1, dumpfile);

    // trailer
    fwrite(&blockTotalLength, sizeof(blockTotalLength), 1, dumpfile);
}

void PcapngWriter::writeSectionHeaderBlock(const PcapngSectionOptions& options)
{
    std::vector<uint8_t> fields;
    appendValue<uint32_t>(fields, PCAP_MAGIC);
    appendValue<uint16_t>(fields, 1); // major version
    appendValue<uint16_t>(fields, 0); // minor version
    appendValue<uint64_t>(fields, -1L); // section length is not specified
    writeBlock(PCAPNG_SECTION_HEADER_BLOCK, fields, nullptr, 0, options);

    // interface IDs are unique only in their section
    hasSection = true;
    interfaces.clear();
    interfaceModuleIdAndLinkTypeToPcapngInterfaceId.clear();
    linkTypeToPcapngInterfaceIdWithoutNetworkInterface.clear();
}

int PcapngWriter::writeInterfaceDescriptionBlock(PcapLinkType linkType, unsigned int snaplen, int timePrecision, const PcapngInterfaceOptions& options)
{
    if (!hasSection)
        throw cRuntimeError("Cannot write interface: the PCAPng file has no section header block");
    for (const auto& option : options.getOptions())
        if (option.code == PCAPNG_IF_TSRESOL)
            throw cRuntimeError("Cannot write interface: the time precision gives the if_tsresol option");
    std::vector<uint8_t> fields;
    appendValue<uint16_t>(fields, linkType);
    appendValue<uint16_t>(fields, 0); // reserved
    appendValue<uint32_t>(fields, snaplen);
    PcapngInterfaceOptions interfaceOptions = options;
    interfaceOptions.addUint8(PCAPNG_IF_TSRESOL, timePrecision);
    writeBlock(PCAPNG_INTERFACE_DESCRIPTION_BLOCK, fields, nullptr, 0, interfaceOptions);
    interfaces.push_back({snaplen, timePrecision});
    return interfaces.size() - 1;
}

void PcapngWriter::writeEnhancedPacketBlock(int interfaceId, simtime_t time, const uint8_t *data, size_t dataLength, size_t originalLength, const PcapngPacketOptions& options)
{
    if (!hasSection)
        throw cRuntimeError("Cannot write packet: the PCAPng file has no section header block");
    if (interfaceId < 0 || interfaceId >= (int)interfaces.size())
        throw cRuntimeError("Cannot write packet: the PCAPng section has no interface %d", interfaceId);
    if (originalLength < dataLength)
        throw cRuntimeError("Cannot write packet: the original length %zu is less than the data length %zu", originalLength, dataLength);
    const auto& interface = interfaces[interfaceId];
    size_t capturedLength = interface.snaplen == 0 ? dataLength : std::min<size_t>(dataLength, interface.snaplen);
    ASSERT(time >= SIMTIME_ZERO);
    uint64_t timestamp = time.inUnit(static_cast<SimTimeUnit>(-interface.timePrecision));
    std::vector<uint8_t> fields;
    appendValue<uint32_t>(fields, interfaceId);
    appendValue<uint32_t>(fields, timestamp >> 32);
    appendValue<uint32_t>(fields, timestamp & 0xFFFFFFFFLLU);
    appendValue<uint32_t>(fields, capturedLength);
    appendValue<uint32_t>(fields, originalLength);
    writeBlock(PCAPNG_ENHANCED_PACKET_BLOCK, fields, data, capturedLength, options);

    if (flush)
        fflush(dumpfile);
}

int PcapngWriter::writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing interface to file" << EV_FIELD(fileName) << EV_FIELD(networkInterface) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write interface: pcap output file is not open");

    PcapngInterfaceOptions options;
    if (networkInterface != nullptr) {
        options.addName(networkInterface->getInterfaceName());
        std::string fullPath = networkInterface->getInterfaceFullPath();
        options.addDescription(fullPath.substr(fullPath.find('.') + 1));
        options.addMacAddress(networkInterface->getMacAddress());
        options.addIpv4Address(networkInterface->getIpv4Address(), networkInterface->getIpv4Netmask());
    }
    return writeInterfaceDescriptionBlock(linkType, snaplen, timePrecision, options);
}

void PcapngWriter::writePacket(simtime_t stime, const Packet *packet, b frontOffset, b backOffset, Direction direction, NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing packet to file" << EV_FIELD(fileName) << EV_FIELD(packet) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    // an interface description has one link type, so each link type of an interface needs its own
    int pcapngInterfaceId;
    if (networkInterface == nullptr) {
        auto it = linkTypeToPcapngInterfaceIdWithoutNetworkInterface.find(linkType);
        if (it != linkTypeToPcapngInterfaceIdWithoutNetworkInterface.end())
            pcapngInterfaceId = it->second;
        else {
            pcapngInterfaceId = writeInterface(nullptr, linkType);
            linkTypeToPcapngInterfaceIdWithoutNetworkInterface[linkType] = pcapngInterfaceId;
        }
    }
    else {
        auto interfaceKey = std::make_pair(networkInterface->getId(), linkType);
        auto it = interfaceModuleIdAndLinkTypeToPcapngInterfaceId.find(interfaceKey);
        if (it != interfaceModuleIdAndLinkTypeToPcapngInterfaceId.end())
            pcapngInterfaceId = it->second;
        else {
            pcapngInterfaceId = writeInterface(networkInterface, linkType);
            interfaceModuleIdAndLinkTypeToPcapngInterfaceId[interfaceKey] = pcapngInterfaceId;
        }
    }

    PcapngPacketOptions options;
    // the receiver of a frame with bit errors finds a wrong frame check sequence
    uint32_t linkLayerErrors = packet->hasBitError() ? PCAPNG_ERROR_CRC : 0;
    if (direction != DIRECTION_UNDEFINED || linkLayerErrors != 0) {
        PcapngPacketFlags flags;
        flags.direction = direction;
        flags.linkLayerErrors = linkLayerErrors;
        options.addFlags(flags);
    }

    b originalLength = packet->getDataLength() - frontOffset - backOffset;
    b capturedLength = snaplen == 0 ? originalLength : std::min(originalLength, b(B(snaplen)));
    if (capturedLength != b(0)) {
        auto data = packet->peekDataAt<BytesChunk>(frontOffset, capturedLength);
        const auto& bytes = data->getBytes();
        writeEnhancedPacketBlock(pcapngInterfaceId, stime, bytes.data(), bytes.size(), originalLength.get<B>(), options);
    }
    else
        writeEnhancedPacketBlock(pcapngInterfaceId, stime, nullptr, 0, originalLength.get<B>(), options);
}

void PcapngWriter::close()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = nullptr;
    }
}

} // namespace inet

