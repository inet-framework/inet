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

void PcapngWriter::open(const char *filename, unsigned int snaplen)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");

    inet::utils::makePathForFile(filename);
    dumpfile = fopen(filename, "wb");
    fileName = filename;

    if (!dumpfile)
        throw cRuntimeError("Cannot open pcap file [%s] for writing: %s", filename, strerror(errno));

    flush = false;

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
    uint32_t optionsLength = (4 + roundUp(name.length())) + (4 + roundUp(fullPath.length())) + (4 + 8) + (4 + 4 + 4) + 4;
    uint32_t blockTotalLength = 20 + optionsLength;
    ASSERT(blockTotalLength % 4 == 0);

    // header
    pcapng_interface_block_header ibh;
    ibh.blockTotalLength = blockTotalLength;
    ibh.linkType = linkType;
    ibh.reserved = 0;
    ibh.snaplen = 0;
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

    // end of options
    uint32_t endOfOptions = 0;
    fwrite(&endOfOptions, sizeof(endOfOptions), 1, dumpfile);

    // trailer
    pcapng_interface_block_trailer ibt;
    ibt.blockTotalLength = blockTotalLength;
    fwrite(&ibt, sizeof(ibt), 1, dumpfile);
}

void PcapngWriter::writePacket(simtime_t stime, const Packet *packet, Direction direction, NetworkInterface *networkInterface, PcapLinkType linkType)
{
    EV_INFO << "Writing packet to file" << EV_FIELD(fileName) << EV_FIELD(packet) << EV_ENDL;
    if (!dumpfile)
        throw cRuntimeError("Cannot write frame: pcap output file is not open");

    auto it = interfaceModuleIdToPcapngInterfaceId.find(networkInterface->getId());
    int pcapngInterfaceId;
    if (it != interfaceModuleIdToPcapngInterfaceId.end())
        pcapngInterfaceId = it->second;
    else {
        writeInterface(networkInterface, linkType);
        pcapngInterfaceId = nextPcapngInterfaceId++;
        interfaceModuleIdToPcapngInterfaceId[networkInterface->getId()] = pcapngInterfaceId;
    }

    if (networkInterface == nullptr)
        throw cRuntimeError("The interface entry not found for packet");

    uint32_t optionsLength = (4 + 4) + 4;
    uint32_t blockTotalLength = 32 + roundUp(packet->getByteLength()) + optionsLength;
    ASSERT(blockTotalLength % 4 == 0);

    // header
    struct pcapng_packet_block_header pbh;
    pbh.blockTotalLength = blockTotalLength;
    pbh.interfaceId = pcapngInterfaceId;
    ASSERT(stime >= SIMTIME_ZERO);
    uint64_t timestamp = stime.inUnit(SIMTIME_US);
    pbh.timestampHigh = static_cast<uint32_t>((timestamp >> 32) & 0xFFFFFFFFLLU);
    pbh.timestampLow = static_cast<uint32_t>(timestamp & 0xFFFFFFFFLLU);
    pbh.capturedPacketLength = packet->getByteLength();
    pbh.originalPacketLength = packet->getByteLength();
    fwrite(&pbh, sizeof(pbh), 1, dumpfile);

    // packet data
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    fwrite(bytes.data(), packet->getByteLength(), 1, dumpfile);

    // packet padding
    char padding[] = { 0, 0, 0, 0 };
    int paddingLength = pad(packet->getByteLength());
    fwrite(padding, paddingLength, 1, dumpfile);

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

