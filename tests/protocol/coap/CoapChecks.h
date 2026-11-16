//
// Independent CoAP wire-byte encoder/decoder and protocol test assertions.
// Does NOT depend on CoapHeaderSerializer for wire verification.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_COAPCHECKS_H
#define __INET_PROTOCOLTEST_COAPCHECKS_H

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "inet/common/MemoryOutputStream.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "PacketEvent.h"
#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

struct RawCoapOption
{
    uint16_t number = 0;
    std::vector<uint8_t> value;
};

struct RawCoapMessage
{
    uint8_t version = 1;
    uint8_t type = 1; // 0=CON, 1=NON, 2=ACK, 3=RST
    std::vector<uint8_t> token;
    uint8_t code = 0;
    uint16_t messageId = 0;
    std::vector<RawCoapOption> options;
    bool hasPayloadMarker = false;
    std::vector<uint8_t> payload;
};

inline std::vector<uint8_t> coapUintBytes(uint32_t value)
{
    if (value == 0)
        return {};
    std::vector<uint8_t> out;
    if (value <= 0xFFU) {
        out.push_back(static_cast<uint8_t>(value));
    }
    else if (value <= 0xFFFFU) {
        out.push_back(static_cast<uint8_t>((value >> 8U) & 0xFFU));
        out.push_back(static_cast<uint8_t>(value & 0xFFU));
    }
    else if (value <= 0xFFFFFFU) {
        out.push_back(static_cast<uint8_t>((value >> 16U) & 0xFFU));
        out.push_back(static_cast<uint8_t>((value >> 8U) & 0xFFU));
        out.push_back(static_cast<uint8_t>(value & 0xFFU));
    }
    else {
        out.push_back(static_cast<uint8_t>((value >> 24U) & 0xFFU));
        out.push_back(static_cast<uint8_t>((value >> 16U) & 0xFFU));
        out.push_back(static_cast<uint8_t>((value >> 8U) & 0xFFU));
        out.push_back(static_cast<uint8_t>(value & 0xFFU));
    }
    return out;
}

inline uint32_t coapDecodedUint(const std::vector<uint8_t>& bytes)
{
    uint32_t val = 0;
    for (uint8_t b : bytes)
        val = (val << 8U) | b;
    return val;
}

inline void encodeRawNibble(uint32_t value, uint8_t& nibble, std::vector<uint8_t>& ext)
{
    ext.clear();
    if (value <= 12U) {
        nibble = static_cast<uint8_t>(value);
    }
    else if (value <= 268U) {
        nibble = 13U;
        ext.push_back(static_cast<uint8_t>(value - 13U));
    }
    else {
        nibble = 14U;
        uint16_t v = static_cast<uint16_t>(value - 269U);
        ext.push_back(static_cast<uint8_t>((v >> 8U) & 0xFFU));
        ext.push_back(static_cast<uint8_t>(v & 0xFFU));
    }
}

inline std::vector<uint8_t> encodeRawCoap(const RawCoapMessage& msg)
{
    std::vector<uint8_t> out;
    uint8_t tkl = static_cast<uint8_t>(msg.token.size());
    out.push_back(static_cast<uint8_t>(((msg.version & 0x03U) << 6U)
            | ((msg.type & 0x03U) << 4U)
            | (tkl & 0x0FU)));
    out.push_back(msg.code);
    out.push_back(static_cast<uint8_t>((msg.messageId >> 8U) & 0xFFU));
    out.push_back(static_cast<uint8_t>(msg.messageId & 0xFFU));

    for (uint8_t b : msg.token)
        out.push_back(b);

    uint16_t prevNum = 0;
    for (const auto& opt : msg.options) {
        uint32_t delta = opt.number - prevNum;
        uint32_t len = static_cast<uint32_t>(opt.value.size());
        uint8_t dNib = 0, lNib = 0;
        std::vector<uint8_t> dExt, lExt;
        encodeRawNibble(delta, dNib, dExt);
        encodeRawNibble(len, lNib, lExt);
        out.push_back(static_cast<uint8_t>((dNib << 4U) | lNib));
        out.insert(out.end(), dExt.begin(), dExt.end());
        out.insert(out.end(), lExt.begin(), lExt.end());
        out.insert(out.end(), opt.value.begin(), opt.value.end());
        prevNum = opt.number;
    }

    if (msg.hasPayloadMarker || !msg.payload.empty()) {
        out.push_back(0xFFU);
        out.insert(out.end(), msg.payload.begin(), msg.payload.end());
    }
    return out;
}

inline bool decodeRawExtended(const std::vector<uint8_t>& bytes, size_t& pos, uint8_t nibble, uint32_t& outVal)
{
    if (nibble <= 12U) {
        outVal = nibble;
        return true;
    }
    if (nibble == 13U) {
        if (pos >= bytes.size())
            return false;
        outVal = static_cast<uint32_t>(bytes[pos++]) + 13U;
        return true;
    }
    if (nibble == 14U) {
        if (pos + 2 > bytes.size())
            return false;
        outVal = (static_cast<uint32_t>(bytes[pos]) << 8U) | bytes[pos + 1];
        outVal += 269U;
        pos += 2;
        return true;
    }
    return false;
}

inline bool decodeRawCoap(const std::vector<uint8_t>& bytes, RawCoapMessage& out)
{
    out = RawCoapMessage();
    if (bytes.size() < 4)
        return false;

    out.version = static_cast<uint8_t>((bytes[0] >> 6U) & 0x03U);
    out.type = static_cast<uint8_t>((bytes[0] >> 4U) & 0x03U);
    uint8_t tkl = static_cast<uint8_t>(bytes[0] & 0x0FU);
    out.code = bytes[1];
    out.messageId = static_cast<uint16_t>((static_cast<uint16_t>(bytes[2]) << 8U) | bytes[3]);

    if (out.version != 1U || tkl > 8U)
        return false;

    size_t pos = 4;
    if (pos + tkl > bytes.size())
        return false;

    out.token.assign(bytes.begin() + pos, bytes.begin() + pos + tkl);
    pos += tkl;

    uint16_t curNum = 0;
    while (pos < bytes.size()) {
        uint8_t b = bytes[pos++];
        if (b == 0xFFU) {
            if (pos >= bytes.size())
                return false;
            out.hasPayloadMarker = true;
            out.payload.assign(bytes.begin() + pos, bytes.end());
            return true;
        }
        uint8_t dNib = static_cast<uint8_t>((b >> 4U) & 0x0FU);
        uint8_t lNib = static_cast<uint8_t>(b & 0x0FU);
        uint32_t delta = 0, len = 0;
        if (!decodeRawExtended(bytes, pos, dNib, delta))
            return false;
        if (!decodeRawExtended(bytes, pos, lNib, len))
            return false;
        uint32_t nextNum = static_cast<uint32_t>(curNum) + delta;
        if (nextNum > 65535U || pos + len > bytes.size())
            return false;
        RawCoapOption opt;
        opt.number = static_cast<uint16_t>(nextNum);
        opt.value.assign(bytes.begin() + pos, bytes.begin() + pos + len);
        pos += len;
        out.options.push_back(std::move(opt));
        curNum = static_cast<uint16_t>(nextNum);
    }
    return true;
}

inline std::vector<uint8_t> extractAppCoapWireBytes(const Packet *packet)
{
    if (packet == nullptr || packet->getDataLength() == B(0))
        return {};
    MemoryOutputStream stream;
    Chunk::serialize(stream, packet->peekData());
    return stream.getData();
}

inline bool extractRawCoapFromAppEvent(const PacketEvent& ev, RawCoapMessage& out)
{
    if (ev.packet == nullptr)
        return false;
    return decodeRawCoap(extractAppCoapWireBytes(ev.packet), out);
}

inline std::vector<std::vector<uint8_t>> getRawOptionValues(const RawCoapMessage& msg, uint16_t number)
{
    std::vector<std::vector<uint8_t>> res;
    for (const auto& opt : msg.options) {
        if (opt.number == number)
            res.push_back(opt.value);
    }
    return res;
}

inline std::vector<std::string> getRawOptionStrings(const RawCoapMessage& msg, uint16_t number)
{
    std::vector<std::string> res;
    for (const auto& opt : msg.options) {
        if (opt.number == number)
            res.emplace_back(opt.value.begin(), opt.value.end());
    }
    return res;
}

inline std::string rawPayloadString(const RawCoapMessage& msg)
{
    return std::string(msg.payload.begin(), msg.payload.end());
}

inline std::string bytesToHexString(const std::vector<uint8_t>& bytes)
{
    std::ostringstream oss;
    for (uint8_t b : bytes)
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b);
    return oss.str();
}

inline std::vector<uint8_t> asciiBytes(const std::string& text)
{
    return std::vector<uint8_t>(text.begin(), text.end());
}

} // namespace protocoltest
} // namespace inet

#endif
