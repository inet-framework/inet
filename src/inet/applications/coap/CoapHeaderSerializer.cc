//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapHeaderSerializer.h"

#include <algorithm>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
namespace coap {

Register_Serializer(CoapHeader, CoapHeaderSerializer);

namespace {

int extendedFieldByteCount(uint32_t value)
{
    if (value <= 12)
        return 0;
    if (value <= 268)
        return 1;
    if (value <= 65804)
        return 2;
    return -1;
}

uint8_t encodeOptionNibble(uint32_t value, uint8_t& ext8, uint16_t& ext16, int& extBytes)
{
    if (value <= 12) {
        extBytes = 0;
        return static_cast<uint8_t>(value);
    }
    if (value <= 268) {
        ext8 = static_cast<uint8_t>(value - 13);
        extBytes = 1;
        return 13;
    }
    if (value <= 65804) {
        ext16 = static_cast<uint16_t>(value - 269);
        extBytes = 2;
        return 14;
    }
    throw cRuntimeError("CoAP option delta or length exceeds 65804: %u", value);
}

bool decodeExtendedValue(MemoryInputStream& stream, uint8_t nibble, uint32_t& result)
{
    if (nibble < 13) {
        result = nibble;
        return true;
    }
    if (nibble == 13) {
        if (stream.getRemainingLength() < B(1))
            return false;
        result = 13U + stream.readByte();
        return true;
    }
    if (nibble == 14) {
        if (stream.getRemainingLength() < B(2))
            return false;
        result = 269U + stream.readUint16Be();
        return true;
    }
    return false;
}

void consumeRemainingBytes(MemoryInputStream& stream)
{
    while (stream.getRemainingLength() >= B(1))
        stream.readByte();
    while (stream.getRemainingLength() > b(0))
        stream.readBit();
}

} // namespace

std::vector<uint8_t> encodeCoapUintValue(uint32_t value)
{
    if (value == 0)
        return {};
    std::vector<uint8_t> bytes;
    for (int shift = 24; shift >= 0; shift -= 8) {
        uint8_t byteVal = static_cast<uint8_t>((value >> static_cast<unsigned int>(shift)) & 0xFFU);
        if (!bytes.empty() || byteVal != 0)
            bytes.push_back(byteVal);
    }
    return bytes;
}

bool decodeCoapUintValue(const CoapOption& option, uint32_t& value, size_t maxBytes)
{
    size_t len = option.getValueArraySize();
    if (len > maxBytes)
        return false;
    uint32_t acc = 0;
    for (size_t i = 0; i < len; ++i)
        acc = (acc << 8U) | option.getValue(i);
    value = acc;
    return true;
}

void addCoapOption(const Ptr<CoapHeader>& header, uint16_t number, const std::vector<uint8_t>& value)
{
    CoapOption option;
    option.setNumber(number);
    option.setValueArraySize(value.size());
    for (size_t i = 0; i < value.size(); ++i)
        option.setValue(i, value[i]);

    // Insert preserving stable ascending order by option number.
    std::vector<CoapOption> items;
    items.reserve(header->getOptionsArraySize() + 1);
    bool inserted = false;
    for (size_t i = 0; i < header->getOptionsArraySize(); ++i) {
        const auto& existing = header->getOptions(i);
        if (!inserted && existing.getNumber() > number) {
            items.push_back(option);
            inserted = true;
        }
        items.push_back(existing);
    }
    if (!inserted)
        items.push_back(option);

    header->setOptionsArraySize(items.size());
    for (size_t i = 0; i < items.size(); ++i)
        header->setOptions(i, items[i]);
}

void addCoapOptionString(const Ptr<CoapHeader>& header, uint16_t number, const std::string& value)
{
    std::vector<uint8_t> bytes(value.begin(), value.end());
    addCoapOption(header, number, bytes);
}

void addCoapOptionUint(const Ptr<CoapHeader>& header, uint16_t number, uint32_t value)
{
    addCoapOption(header, number, encodeCoapUintValue(value));
}

void removeCoapOptions(const Ptr<CoapHeader>& header, uint16_t number)
{
    std::vector<CoapOption> kept;
    kept.reserve(header->getOptionsArraySize());
    for (size_t i = 0; i < header->getOptionsArraySize(); ++i) {
        const auto& opt = header->getOptions(i);
        if (opt.getNumber() != number)
            kept.push_back(opt);
    }
    header->setOptionsArraySize(kept.size());
    for (size_t i = 0; i < kept.size(); ++i)
        header->setOptions(i, kept[i]);
}

void setCoapOptionUint(const Ptr<CoapHeader>& header, uint16_t number, uint32_t value)
{
    removeCoapOptions(header, number);
    addCoapOptionUint(header, number, value);
}

B computeCoapHeaderLength(const CoapHeader& header)
{
    size_t total = 4 + header.getTokenArraySize();
    std::vector<size_t> order(header.getOptionsArraySize());
    for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return header.getOptions(a).getNumber() < header.getOptions(b).getNumber();
    });

    uint16_t prevNumber = 0;
    for (size_t idx : order) {
        const auto& opt = header.getOptions(idx);
        uint32_t delta = opt.getNumber() - prevNumber;
        uint32_t optLen = static_cast<uint32_t>(opt.getValueArraySize());
        int deltaExt = extendedFieldByteCount(delta);
        int lenExt = extendedFieldByteCount(optLen);
        if (deltaExt < 0 || lenExt < 0)
            throw cRuntimeError("CoAP option delta or length exceeds wire encoding limit");
        total += 1 + static_cast<size_t>(deltaExt) + static_cast<size_t>(lenExt) + optLen;
        prevNumber = opt.getNumber();
    }
    if (header.getPayloadMarkerPresent())
        total += 1;
    return B(total);
}

void finalizeCoapHeader(const Ptr<CoapHeader>& header, bool hasPayload)
{
    header->setPayloadMarkerPresent(hasPayload);
    header->setChunkLength(computeCoapHeaderLength(*header));
}

void CoapHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const CoapHeader>(chunk);
    if (header->getVersion() != 1)
        throw cRuntimeError("Cannot serialize CoAP header with unsupported version %u", header->getVersion());
    size_t tkl = header->getTokenArraySize();
    if (tkl > 8)
        throw cRuntimeError("Cannot serialize CoAP header with token length %zu > 8", tkl);

    uint8_t firstByte = static_cast<uint8_t>(((header->getVersion() & 0x03U) << 6U)
            | ((static_cast<uint8_t>(header->getMessageType()) & 0x03U) << 4U)
            | (static_cast<uint8_t>(tkl) & 0x0FU));
    stream.writeByte(firstByte);
    stream.writeByte(header->getCode());
    stream.writeUint16Be(header->getMessageId());

    for (size_t i = 0; i < tkl; ++i)
        stream.writeByte(header->getToken(i));

    std::vector<size_t> order(header->getOptionsArraySize());
    for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return header->getOptions(a).getNumber() < header->getOptions(b).getNumber();
    });

    uint16_t prevNumber = 0;
    for (size_t idx : order) {
        const auto& opt = header->getOptions(idx);
        uint32_t delta = opt.getNumber() - prevNumber;
        uint32_t optLen = static_cast<uint32_t>(opt.getValueArraySize());
        uint8_t deltaExt8 = 0;
        uint16_t deltaExt16 = 0;
        int deltaExtBytes = 0;
        uint8_t lenExt8 = 0;
        uint16_t lenExt16 = 0;
        int lenExtBytes = 0;
        uint8_t deltaNibble = encodeOptionNibble(delta, deltaExt8, deltaExt16, deltaExtBytes);
        uint8_t lenNibble = encodeOptionNibble(optLen, lenExt8, lenExt16, lenExtBytes);

        stream.writeByte(static_cast<uint8_t>((deltaNibble << 4U) | lenNibble));
        if (deltaExtBytes == 1)
            stream.writeByte(deltaExt8);
        else if (deltaExtBytes == 2)
            stream.writeUint16Be(deltaExt16);

        if (lenExtBytes == 1)
            stream.writeByte(lenExt8);
        else if (lenExtBytes == 2)
            stream.writeUint16Be(lenExt16);

        for (size_t j = 0; j < optLen; ++j)
            stream.writeByte(opt.getValue(j));
        prevNumber = opt.getNumber();
    }

    if (header->getPayloadMarkerPresent())
        stream.writeByte(0xFF);
}

const Ptr<Chunk> CoapHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<CoapHeader>();
    auto failAndConsume = [&]() -> const Ptr<Chunk> {
        header->markIncorrect();
        header->setPayloadMarkerPresent(false);
        consumeRemainingBytes(stream);
        return header;
    };

    if (stream.getRemainingLength() < B(4))
        return failAndConsume();

    uint8_t byte0 = stream.readByte();
    uint8_t ver = static_cast<uint8_t>((byte0 >> 6U) & 0x03U);
    uint8_t typeVal = static_cast<uint8_t>((byte0 >> 4U) & 0x03U);
    uint8_t tkl = static_cast<uint8_t>(byte0 & 0x0FU);
    uint8_t code = stream.readByte();
    uint16_t mid = stream.readUint16Be();

    header->setVersion(ver);
    header->setMessageType(static_cast<CoapMessageType>(typeVal));
    header->setCode(code);
    header->setMessageId(mid);

    if (ver != 1 || tkl > 8 || isCoapCodeReservedClass(code))
        return failAndConsume();

    if (code == COAP_CODE_EMPTY) {
        if (tkl != 0 || stream.getRemainingLength() != b(0))
            return failAndConsume();
        return header;
    }

    if (stream.getRemainingLength() < B(tkl))
        return failAndConsume();

    header->setTokenArraySize(tkl);
    for (uint8_t i = 0; i < tkl; ++i)
        header->setToken(i, stream.readByte());

    uint16_t currentNumber = 0;
    while (stream.getRemainingLength() >= B(1)) {
        uint8_t optByte = stream.readByte();
        if (optByte == 0xFF) {
            if (stream.getRemainingLength() <= b(0))
                return failAndConsume();
            header->setPayloadMarkerPresent(true);
            return header;
        }

        uint8_t deltaNibble = static_cast<uint8_t>((optByte >> 4U) & 0x0FU);
        uint8_t lenNibble = static_cast<uint8_t>(optByte & 0x0FU);
        if (deltaNibble == 15 || lenNibble == 15)
            return failAndConsume();

        uint32_t delta = 0;
        if (!decodeExtendedValue(stream, deltaNibble, delta))
            return failAndConsume();

        uint32_t optLen = 0;
        if (!decodeExtendedValue(stream, lenNibble, optLen))
            return failAndConsume();

        uint32_t nextNumber = static_cast<uint32_t>(currentNumber) + delta;
        if (nextNumber > 65535U)
            return failAndConsume();

        if (stream.getRemainingLength() < B(optLen))
            return failAndConsume();

        CoapOption opt;
        opt.setNumber(static_cast<uint16_t>(nextNumber));
        opt.setValueArraySize(optLen);
        for (uint32_t i = 0; i < optLen; ++i)
            opt.setValue(i, stream.readByte());
        header->appendOptions(opt);
        currentNumber = static_cast<uint16_t>(nextNumber);
    }

    if (stream.getRemainingLength() > b(0))
        return failAndConsume();

    return header;
}

bool validateCoapHeaderStructure(const CoapHeader& header, size_t payloadSize)
{
    if (header.getVersion() != 1)
        return false;
    size_t tkl = header.getTokenArraySize();
    if (tkl > 8 || isCoapCodeReservedClass(header.getCode()))
        return false;
    if (header.getCode() == COAP_CODE_EMPTY) {
        if (tkl != 0 || header.getOptionsArraySize() != 0 || header.getPayloadMarkerPresent() || payloadSize != 0)
            return false;
        return header.getMessageType() != COAP_TYPE_NON;
    }
    if (header.getMessageType() == COAP_TYPE_RST)
        return false; // RFC 7252 §4.1: Reset messages MUST be empty (code 0.00).
    if (header.getPayloadMarkerPresent() != (payloadSize > 0))
        return false;
    uint16_t prev = 0;
    for (size_t i = 0; i < header.getOptionsArraySize(); ++i) {
        uint16_t num = header.getOptions(i).getNumber();
        if (num < prev)
            return false;
        prev = num;
    }
    return true;
}

bool parseCoapDatagramBytes(const std::vector<uint8_t>& bytes, Ptr<CoapHeader>& header, std::vector<uint8_t>& payload)
{
    payload.clear();
    if (bytes.size() < 4 || ((bytes[0] >> 6U) & 0x03U) != 1U) {
        header = nullptr;
        return false;
    }

    MemoryInputStream stream(bytes);
    CoapHeaderSerializer serializer;
    auto chunk = serializer.deserialize(stream);
    header = dynamicPtrCast<CoapHeader>(chunk);
    if (header == nullptr || header->isIncorrect())
        return false;

    size_t remainingBytes = static_cast<size_t>(B(stream.getRemainingLength()).get());
    if (remainingBytes > 0) {
        payload.resize(remainingBytes);
        for (size_t i = 0; i < remainingBytes; ++i)
            payload[i] = stream.readByte();
    }

    return validateCoapHeaderStructure(*header, payload.size());
}

} // namespace coap
} // namespace inet
