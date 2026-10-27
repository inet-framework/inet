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

void PcapngOptions::addOption(uint16_t code, const std::vector<uint8_t>& value)
{
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

PcapngWriter::~PcapngWriter()
{
    PcapngWriter::close(); // NOTE: admitting that this will not call overridden methods from the destructor
}

void PcapngWriter::open(const char *filename, unsigned int snaplen, int timePrecision)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    inet::utils::makePathForFile(filename);
    dumpfile = fopen(filename, "wb");
    fileName = filename;

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    flush = false;
    this->snaplen = snaplen;

    // TODO check validity of timePrecision
    this->timePrecision = timePrecision;

    writeSectionHeaderBlock(PcapngOptions());
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

void PcapngWriter::writeSectionHeaderBlock(const PcapngOptions& options)
{
    std::vector<uint8_t> fields;
    appendValue<uint32_t>(fields, PCAP_MAGIC);
    appendValue<uint16_t>(fields, 1); // major version
    appendValue<uint16_t>(fields, 0); // minor version
    appendValue<uint64_t>(fields, -1L); // section length is not specified
    writeBlock(0x0A0D0D0A, fields, nullptr, 0, options);

    // interface IDs are unique only in their section
    interfaces.clear();
    interfaceModuleIdAndLinkTypeToPcapngInterfaceId.clear();
    linkTypeToPcapngInterfaceIdWithoutNetworkInterface.clear();
}

int PcapngWriter::writeInterfaceDescriptionBlock(PcapLinkType linkType, unsigned int snaplen, int timePrecision, const PcapngOptions& options)
{
    std::vector<uint8_t> fields;
    appendValue<uint16_t>(fields, linkType);
    appendValue<uint16_t>(fields, 0); // reserved
    appendValue<uint32_t>(fields, snaplen);
    PcapngOptions interfaceOptions = options;
    interfaceOptions.addUint8(0x0009, timePrecision); // tsresol
    writeBlock(0x00000001, fields, nullptr, 0, interfaceOptions);
    interfaces.push_back({snaplen, timePrecision});
    return interfaces.size() - 1;
}

void PcapngWriter::writeEnhancedPacketBlock(int interfaceId, simtime_t time, const uint8_t *data, size_t dataLength, size_t originalLength, const PcapngOptions& options)
{
    const auto& interface = interfaces.at(interfaceId);
    size_t capturedLength = interface.snaplen == 0 ? dataLength : std::min<size_t>(dataLength, interface.snaplen);
    ASSERT(time >= SIMTIME_ZERO);
    uint64_t timestamp = time.inUnit(static_cast<SimTimeUnit>(-interface.timePrecision));
    std::vector<uint8_t> fields;
    appendValue<uint32_t>(fields, interfaceId);
    appendValue<uint32_t>(fields, timestamp >> 32);
    appendValue<uint32_t>(fields, timestamp & 0xFFFFFFFFLLU);
    appendValue<uint32_t>(fields, capturedLength);
    appendValue<uint32_t>(fields, originalLength);
    writeBlock(0x00000006, fields, data, capturedLength, options);

    if (flush)
        fflush(dumpfile);
}

int PcapngWriter::writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing interface to file" << EV_FIELD(fileName) << EV_FIELD(networkInterface) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write interface: pcap output file is not open");

    PcapngOptions options;
    if (networkInterface != nullptr) {
        // interface name option
        options.addString(0x0002, networkInterface->getInterfaceName());

        // interface description option
        std::string fullPath = networkInterface->getInterfaceFullPath();
        options.addString(0x0003, fullPath.substr(fullPath.find('.') + 1));

        // MAC address option
        uint8_t macAddressBytes[6];
        networkInterface->getMacAddress().getAddressBytes(macAddressBytes);
        options.addOption(0x0006, std::vector<uint8_t>(macAddressBytes, macAddressBytes + 6));

        // IP address/netmask option
        std::vector<uint8_t> ipAddressBytes;
        auto ipv4Address = networkInterface->getIpv4Address();
        for (int i = 0; i < 4; i++) ipAddressBytes.push_back(ipv4Address.getDByte(i));
        auto ipv4Netmask = networkInterface->getIpv4Netmask();
        for (int i = 0; i < 4; i++) ipAddressBytes.push_back(ipv4Netmask.getDByte(i));
        options.addOption(0x0004, ipAddressBytes);
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

    // direction option
    PcapngOptions options;
    uint32_t flagsOptionValue = 0;
    switch (direction) {
        case DIRECTION_INBOUND:
            flagsOptionValue = 0b01;
            break;
        case DIRECTION_OUTBOUND:
            flagsOptionValue = 0b10;
            break;
        default:
            throw cRuntimeError("Unknown direction value");
    }
    options.addUint32(0x0002, flagsOptionValue);

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

