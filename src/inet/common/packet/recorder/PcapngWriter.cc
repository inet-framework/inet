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

struct pcapng_option_header
{
    uint16_t code;
    uint16_t length;
};

struct pcapng_section_block_header
{
    uint32_t blockType = 0x0A0D0D0A;
    uint32_t blockTotalLength;
    uint32_t byteOrderMagic;
    uint16_t majorVersion;
    uint16_t minorVersion;
    uint64_t sectionLength;
};

struct pcapng_section_block_trailer
{
    uint32_t blockTotalLength;
};

struct pcapng_interface_block_header
{
    uint32_t blockType = 0x00000001;
    uint32_t blockTotalLength;
    uint16_t linkType;
    uint16_t reserved;
    uint32_t snaplen;
};

struct pcapng_interface_block_trailer
{
    uint32_t blockTotalLength;
};

struct pcapng_packet_block_header
{
    uint32_t blockType = 0x00000006;
    uint32_t blockTotalLength;
    uint32_t interfaceId;
    uint32_t timestampHigh;
    uint32_t timestampLow;
    uint32_t capturedPacketLength;
    uint32_t originalPacketLength;
};

struct pcapng_packet_block_trailer
{
    uint32_t blockTotalLength;
};

static int pad(int value, int multiplier = 4)
{
    return (multiplier - value % multiplier) % multiplier;
}

static int roundUp(int value, int multiplier = 4)
{
    return value + pad(value, multiplier);
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
    nextPcapngInterfaceId = 0;
    interfaceModuleIdToPcapngInterface.clear();
    this->snaplen = snaplen;

    // TODO check validity of timePrecision
    this->timePrecision = timePrecision;

    // header
    int blockTotalLength = 28;
    ASSERT(blockTotalLength % 4 == 0);
    struct pcapng_section_block_header sbh;
    sbh.blockTotalLength = blockTotalLength;
    sbh.byteOrderMagic = PCAP_MAGIC;
    sbh.majorVersion = 1;
    sbh.minorVersion = 0;
    sbh.sectionLength = -1L;
    fwrite(&sbh, sizeof(sbh), 1, dumpfile);

    // trailer
    struct pcapng_section_block_trailer sbt;
    sbt.blockTotalLength = blockTotalLength;
    fwrite(&sbt, sizeof(sbt), 1, dumpfile);
}

void PcapngWriter::writeInterface(NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing interface to file" << EV_FIELD(fileName) << EV_FIELD(networkInterface) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write interface: pcap output file is not open");

    std::string name = networkInterface->getInterfaceName();
    std::string fullPath = networkInterface->getInterfaceFullPath();
    fullPath = fullPath.substr(fullPath.find('.') + 1);
    uint32_t optionsLength = (4 + roundUp(name.length())) + (4 + roundUp(fullPath.length())) + (4 + 8) + (4 + 4 + 4) + (4 + 4) + 4;
    uint32_t blockTotalLength = 20 + optionsLength;
    ASSERT(blockTotalLength % 4 == 0);

    // header
    pcapng_interface_block_header ibh;
    ibh.blockTotalLength = blockTotalLength;
    ibh.linkType = linkType;
    ibh.reserved = 0;
    ibh.snaplen = snaplen;
    fwrite(&ibh, sizeof(ibh), 1, dumpfile);

    // interface name option
    pcapng_option_header doh;
    doh.code = 0x0002;
    doh.length = name.length();
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    fwrite(name.c_str(), name.length(), 1, dumpfile);
    char padding[] = { 0, 0, 0, 0 };
    int paddingLength = pad(name.length());
    fwrite(padding, paddingLength, 1, dumpfile);

    // interface description option
    doh.code = 0x0003;
    doh.length = fullPath.length();
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    fwrite(fullPath.c_str(), fullPath.length(), 1, dumpfile);
    paddingLength = pad(fullPath.length());
    fwrite(padding, paddingLength, 1, dumpfile);

    // MAC address option
    doh.code = 0x0006;
    doh.length = 6;
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    uint8_t macAddressBytes[6];
    networkInterface->getMacAddress().getAddressBytes(macAddressBytes);
    fwrite(macAddressBytes, 6, 1, dumpfile);
    fwrite(padding, 2, 1, dumpfile);

    // IP address/netmask option
    doh.code = 0x0004;
    doh.length = 4 + 4;
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    uint8_t ipAddressBytes[4];
    auto ipv4Address = networkInterface->getIpv4Address();
    for (int i = 0; i < 4; i++) ipAddressBytes[i] = ipv4Address.getDByte(i);
    fwrite(ipAddressBytes, 4, 1, dumpfile);
    auto ipv4Netmask = networkInterface->getIpv4Netmask();
    for (int i = 0; i < 4; i++) ipAddressBytes[i] = ipv4Netmask.getDByte(i);
    fwrite(ipAddressBytes, 4, 1, dumpfile);

    // tsresol option
    doh.code = 0x0009;
    doh.length = 1;
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    uint8_t d = timePrecision;
    fwrite(&d, 1, 1, dumpfile);
    paddingLength = pad(1);
    fwrite(padding, paddingLength, 1, dumpfile);

    // end of options
    uint32_t endOfOptions = 0;
    fwrite(&endOfOptions, sizeof(endOfOptions), 1, dumpfile);

    // trailer
    pcapng_interface_block_trailer ibt;
    ibt.blockTotalLength = blockTotalLength;
    fwrite(&ibt, sizeof(ibt), 1, dumpfile);
}

void PcapngWriter::writePacket(simtime_t stime, const Packet *packet, b frontOffset, b backOffset, Direction direction, NetworkInterface *networkInterface, PcapLinkType linkType)
{
    writePacketWithPrefix(stime, {}, packet, frontOffset, backOffset, direction, networkInterface, linkType);
}

void PcapngWriter::writePacketWithPrefix(simtime_t stime, const std::vector<uint8_t>& prefix, const Packet *packet, b frontOffset, b backOffset,
        Direction direction, NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing packet to file" << EV_FIELD(fileName) << EV_FIELD(packet) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    // Enhanced Packet Blocks refer to an Interface Description Block, unlike classic PCAP
    // records. Fail explicitly when no interface can be resolved instead of dereferencing null.
    if (networkInterface == nullptr)
        throw cRuntimeError("The interface entry not found for packet");

    auto it = interfaceModuleIdToPcapngInterface.find(networkInterface->getId());
    int pcapngInterfaceId;
    if (it != interfaceModuleIdToPcapngInterface.end()) {
        if (it->second.second != linkType)
            throw cRuntimeError("linktype mismatch error: required linktype = %d, arrived linktype = %d", it->second.second, linkType);
        pcapngInterfaceId = it->second.first;
    }
    else {
        writeInterface(networkInterface, linkType);
        pcapngInterfaceId = nextPcapngInterfaceId++;
        interfaceModuleIdToPcapngInterface[networkInterface->getId()] = {pcapngInterfaceId, linkType};
    }

    b packetLength = packet->getDataLength() - frontOffset - backOffset;
    size_t originalLength = prefix.size() + packetLength.get<B>();
    // Advertise and enforce the configured snaplen for PCAPng too. A zero snaplen means unlimited;
    // otherwise the captured length is truncated while the original length remains unchanged.
    size_t capturedLength = snaplen == 0 ? originalLength : std::min<size_t>(originalLength, snaplen);
    uint32_t optionsLength = (4 + 4) + 4;
    uint32_t blockTotalLength = 32 + roundUp(capturedLength) + optionsLength;
    ASSERT(blockTotalLength % 4 == 0);

    // header
    struct pcapng_packet_block_header pbh;
    pbh.blockTotalLength = blockTotalLength;
    pbh.interfaceId = pcapngInterfaceId;
    ASSERT(stime >= SIMTIME_ZERO);
    uint64_t timestamp = stime.inUnit(static_cast<SimTimeUnit>(-timePrecision));
    pbh.timestampHigh = static_cast<uint32_t>((timestamp >> 32) & 0xFFFFFFFFLLU);
    pbh.timestampLow = static_cast<uint32_t>(timestamp & 0xFFFFFFFFLLU);
    pbh.capturedPacketLength = capturedLength;
    pbh.originalPacketLength = originalLength;
    fwrite(&pbh, sizeof(pbh), 1, dumpfile);

    if (capturedLength != 0) {
        // packet data
        auto capturedPrefixLength = std::min(prefix.size(), capturedLength);
        if (capturedPrefixLength != 0)
            fwrite(prefix.data(), capturedPrefixLength, 1, dumpfile);
        auto capturedPacketLength = capturedLength - capturedPrefixLength;
        if (capturedPacketLength != 0) {
            auto data = packet->peekDataAt<BytesChunk>(frontOffset, B(capturedPacketLength));
            const auto& bytes = data->getBytes();
            fwrite(bytes.data(), bytes.size(), 1, dumpfile);
        }

        // packet padding
        char padding[] = { 0, 0, 0, 0 };
        int paddingLength = pad(capturedLength);
        fwrite(padding, paddingLength, 1, dumpfile);
    }

    // direction option
    pcapng_option_header doh;
    doh.code = 0x0002;
    doh.length = 4;
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
    fwrite(&doh, sizeof(doh), 1, dumpfile);
    fwrite(&flagsOptionValue, sizeof(flagsOptionValue), 1, dumpfile);

    // end of options
    uint32_t endOfOptions = 0;
    fwrite(&endOfOptions, sizeof(endOfOptions), 1, dumpfile);

    // trailer
    struct pcapng_packet_block_trailer pbt;
    pbt.blockTotalLength = blockTotalLength;
    fwrite(&pbt, sizeof(pbt), 1, dumpfile);

    if (flush)
        fflush(dumpfile);
}

void PcapngWriter::close()
{
    if (dumpfile) {
        fclose(dumpfile);
        dumpfile = nullptr;
    }
}

} // namespace inet
