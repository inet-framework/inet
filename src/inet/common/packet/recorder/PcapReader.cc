//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/recorder/PcapReader.h"

#include <cerrno>

#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

static uint16_t swapByteOrder16(uint16_t v)
{
    return ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8);
}

static uint32_t swapByteOrder32(uint32_t v)
{
    return ((v & 0xFFL) << 24) | ((v & 0xFF00L) << 8) | ((v & 0xFF0000L) >> 8) | ((v & 0xFF000000L) >> 24);
}

void PcapReader::openPcap(const char *filename, const char *packetNameFormat)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");
    this->packetNameFormat = packetNameFormat;
    inet::utils::makePathForFile(filename);
    file = fopen(filename, "rb");
    if (file == nullptr)
        throw cRuntimeError("Cannot open pcap file [%s] for reading: %s", filename, strerror(errno));
    struct pcap_hdr fh;
    auto err = fread(&fh, sizeof(fh), 1, file);
    if (err != 1)
        throw cRuntimeError("Cannot read fileheader from file '%s', errno is %zu.", filename, err);
    if (fh.magic == 0xa1b2c3d4)
        swapByteOrder = false;
    else if (fh.magic == 0xd4c3b2a1)
        swapByteOrder = true;
    else
        throw cRuntimeError("Unknown fileheader read from file '%s'", filename);
    if (swapByteOrder) {
        fh.version_major = swapByteOrder16(fh.version_major);
        fh.version_minor = swapByteOrder16(fh.version_minor);
        fh.thiszone = swapByteOrder32(fh.thiszone);
        fh.sigfigs = swapByteOrder32(fh.sigfigs);
        fh.snaplen = swapByteOrder32(fh.snaplen);
        fh.network = swapByteOrder32(fh.network);
    }
}

std::pair<simtime_t, Packet *> PcapReader::readPacket()
{
    if (file == nullptr)
        throw cRuntimeError("Cannot read packet: pcap input file is not open");
    if (feof(file))
        return { -1, nullptr };
    struct pcaprec_hdr packetHeader;
    auto err = fread(&packetHeader, sizeof(packetHeader), 1, file);
    if (err != 1)
        throw cRuntimeError("Cannot read packetheader, errno is %zu.", err);
    uint8_t buffer[1 << 16];
    memset(buffer, 0, sizeof(buffer));
    err = fread(buffer, packetHeader.incl_len, 1, file);
    if (err != 1)
        throw cRuntimeError("Cannot read packet, errno is %zu.", err);
    if (swapByteOrder) {
        packetHeader.ts_sec = swapByteOrder32(packetHeader.ts_sec);
        packetHeader.ts_usec = swapByteOrder32(packetHeader.ts_usec);
        packetHeader.orig_len = swapByteOrder32(packetHeader.orig_len);
        packetHeader.incl_len = swapByteOrder32(packetHeader.incl_len);
    }
    simtime_t time = SimTime(packetHeader.ts_sec, SIMTIME_S) + SimTime(packetHeader.ts_usec, SIMTIME_US);
    std::vector<uint8_t> bytes;
    const Protocol *protocol = nullptr;
    int offset = 0;
    switch (fileHeader.network) {
        case 0: {
            offset = 4;
            auto type = *((uint32_t *)buffer);
            switch (type) {
                case 2: protocol = &Protocol::ipv4; break;
                case 24: case 28: case 30: protocol = &Protocol::ipv6; break;
            }
            break;
        }
        case 1: protocol = &Protocol::ethernetMac; break;
        case 105: protocol = &Protocol::ieee80211Mac; break;
        case 204: protocol = &Protocol::ppp; break;
    }
    bytes.resize(packetHeader.orig_len - offset);
    for (size_t i = 0; i < bytes.size(); i++)
        bytes[i] = buffer[i + offset];
    const auto& data = makeShared<BytesChunk>(bytes);
    if (packetHeader.orig_len > packetHeader.incl_len)
        data->markIncomplete();
    auto packet = new Packet(nullptr, data);
    if (protocol != nullptr)
        packet->addTag<PacketProtocolTag>()->setProtocol(protocol);
    packet->setName(packetPrinter.printPacketToString(packet, packetNameFormat).c_str());
    return { time, packet };
}

void PcapReader::closePcap()
{
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
}

} // namespace inet

