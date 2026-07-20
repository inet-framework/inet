//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/recorder/PcapReader.h"

#include <cerrno>

#include "inet/common/FcsInd_m.h"
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

// Read a little-endian 32-bit word from a radiotap header (radiotap is always
// little-endian, regardless of the pcap byte order).
static uint32_t radiotapReadLe32(const uint8_t *buf, size_t pos)
{
    return buf[pos] | (buf[pos + 1] << 8) | (buf[pos + 2] << 16) | ((uint32_t)buf[pos + 3] << 24);
}

// Whether a radiotap-encapsulated 802.11 frame still carries its FCS, i.e. the
// "FCS at end" flag (0x10) of the radiotap Flags field is set. The Flags field
// (it_present bit 1) is located after the it_present bitmap words and the TSFT
// field (bit 0), honouring radiotap's field alignment. Returns false if the
// Flags field is absent or the header is truncated.
static bool radiotapHasFcs(const uint8_t *buf, uint32_t caplen)
{
    if (caplen < 8)
        return false;
    uint16_t itLen = buf[2] | (buf[3] << 8);
    uint32_t present0 = radiotapReadLe32(buf, 4);
    // skip any extended it_present words (each with bit 31 set chains the next)
    size_t pos = 4;
    for (uint32_t w = present0; (w & 0x80000000u) != 0; ) {
        pos += 4;
        if (pos + 4 > itLen || pos + 4 > caplen)
            return false;
        w = radiotapReadLe32(buf, pos);
    }
    pos += 4; // data fields begin after the last it_present word
    if (present0 & (1u << 0)) { // TSFT: 8-byte aligned, 8 bytes
        pos = (pos + 7) & ~((size_t)7);
        pos += 8;
    }
    if (present0 & (1u << 1)) { // Flags: 1-byte aligned, 1 byte
        if (pos >= itLen || pos >= caplen)
            return false;
        return (buf[pos] & 0x10) != 0;
    }
    return false;
}

void PcapReader::openPcap(const char *filename, const char *packetNameFormat)
{
    if (opp_isempty(filename))
        throw cRuntimeError("Cannot open pcap file: file name is empty");
    this->packetNameFormat = packetNameFormat;
    file = fopen(filename, "rb");
    if (file == nullptr)
        throw cRuntimeError("Cannot open pcap file [%s] for reading: %s", filename, strerror(errno));
    auto err = fread(&fileHeader, sizeof(fileHeader), 1, file);
    if (err != 1)
        throw cRuntimeError("Cannot read fileheader from file '%s', errno is %u.", filename, (unsigned int)err);
    if (fileHeader.magic == 0xa1b2c3d4)
        swapByteOrder = false;
    else if (fileHeader.magic == 0xd4c3b2a1)
        swapByteOrder = true;
    else
        throw cRuntimeError("Unknown fileheader read from file '%s'", filename);
    if (swapByteOrder) {
        fileHeader.version_major = swapByteOrder16(fileHeader.version_major);
        fileHeader.version_minor = swapByteOrder16(fileHeader.version_minor);
        fileHeader.thiszone = swapByteOrder32(fileHeader.thiszone);
        fileHeader.sigfigs = swapByteOrder32(fileHeader.sigfigs);
        fileHeader.snaplen = swapByteOrder32(fileHeader.snaplen);
        fileHeader.network = swapByteOrder32(fileHeader.network);
    }
}

std::pair<PcapRecordTime, Packet *> PcapReader::readPacket()
{
    if (file == nullptr)
        throw cRuntimeError("Cannot read packet: pcap input file is not open");
    if (feof(file))
        return { PcapRecordTime(), nullptr };
    struct pcaprec_hdr packetHeader;
    auto err = fread(&packetHeader, sizeof(packetHeader), 1, file);
    if (err != 1) {
        if (feof(file)) // clean end of file: no more records
            return { PcapRecordTime(), nullptr };
        throw cRuntimeError("Cannot read packetheader, errno is %u.", (unsigned int)err);
    }
    uint8_t buffer[1 << 16];
    memset(buffer, 0, sizeof(buffer));
    err = fread(buffer, packetHeader.incl_len, 1, file);
    if (err != 1)
        throw cRuntimeError("Cannot read packet, errno is %u.", (unsigned int)err);
    if (swapByteOrder) {
        packetHeader.ts_sec = swapByteOrder32(packetHeader.ts_sec);
        packetHeader.ts_usec = swapByteOrder32(packetHeader.ts_usec);
        packetHeader.orig_len = swapByteOrder32(packetHeader.orig_len);
        packetHeader.incl_len = swapByteOrder32(packetHeader.incl_len);
    }
    PcapRecordTime time { packetHeader.ts_sec, packetHeader.ts_usec };
    std::vector<uint8_t> bytes;
    const Protocol *protocol = nullptr;
    int offset = 0;
    // Some link types tell us whether the frame carries its FCS; for those, set
    // fcsKnown and record hasFcs so an FcsInd tag can be attached (see below).
    bool fcsKnown = false;
    bool hasFcs = false;
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
        case 101: // DLT_RAW: a bare IP packet with no link-layer header; the IP
            // version nibble tells IPv4 from IPv6
            protocol = ((buffer[0] >> 4) == 6) ? &Protocol::ipv6 : &Protocol::ipv4;
            break;
        case 228: protocol = &Protocol::ipv4; break; // DLT_IPV4
        case 229: protocol = &Protocol::ipv6; break; // DLT_IPV6
        case 105: protocol = &Protocol::ieee80211Mac; break;
        case 195: protocol = &Protocol::ieee802154; fcsKnown = true; hasFcs = true; break; // DLT_IEEE802_15_4_WITHFCS
        case 230: protocol = &Protocol::ieee802154; fcsKnown = true; hasFcs = false; break; // DLT_IEEE802_15_4_NOFCS
        case 127: {
            // DLT_IEEE802_11_RADIO: a variable-length radiotap header precedes the
            // 802.11 MAC frame. Its total length is in bytes 2-3 (it_len), stored
            // little-endian regardless of the pcap byte order -- strip it to reach
            // the MAC frame. The radiotap Flags field tells per frame whether the FCS
            // is present.
            offset = buffer[2] | (buffer[3] << 8);
            protocol = &Protocol::ieee80211Mac;
            fcsKnown = true;
            hasFcs = radiotapHasFcs(buffer, packetHeader.incl_len);
            break;
        }
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
    if (fcsKnown)
        packet->addTag<FcsInd>()->setHasFcs(hasFcs);
    // naming a packet requires dissecting it; only do so when a format was requested
    if (packetNameFormat != nullptr)
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

