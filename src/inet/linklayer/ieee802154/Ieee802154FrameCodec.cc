// SPDX-License-Identifier: LGPL-3.0-or-later
//
#include "inet/linklayer/ieee802154/Ieee802154FrameCodec.h"

namespace inet {

namespace {
int addressMode(const Ieee802154Address& address)
{
    return address.isNone() ? 0 : address.isShort() ? 2 : 3;
}

void appendLe(std::vector<uint8_t>& bytes, uint64_t value, unsigned int length)
{
    for (unsigned int i = 0; i < length; ++i)
        bytes.push_back(value >> (8 * i));
}

void appendAddress(std::vector<uint8_t>& bytes, const Ieee802154Address& address)
{
    if (address.isShort())
        appendLe(bytes, address.getShort(), 2);
    else if (address.isExtended())
        appendLe(bytes, address.getExtended(), 8);
}
}

uint16_t Ieee802154FrameCodec::calculateFcs(const std::vector<uint8_t>& bytes)
{
    // IEEE 802.15.4-2024, 7.2.11: zero-initialized CRC, LSB-first octets.
    uint16_t remainder = 0;
    for (auto byte : bytes) {
        remainder ^= byte;
        for (int bit = 0; bit < 8; ++bit)
            remainder = (remainder >> 1) ^ ((remainder & 1) ? 0x8408 : 0);
    }
    return remainder;
}

std::vector<uint8_t> Ieee802154FrameCodec::encode(const Ieee802154Frame& frame)
{
    if (frame.getDecodeStatus() != OK) {
        std::vector<uint8_t> raw;
        for (size_t i = 0; i < frame.getRawBytesArraySize(); ++i)
            raw.push_back(frame.getRawBytes(i));
        return raw;
    }
    if (frame.getFrameVersion() < 0 || frame.getFrameVersion() > 1 ||
            frame.getFrameType() < 0 || frame.getFrameType() > 3)
        throw cRuntimeError("Unsupported IEEE 802.15.4 transmit frame/version");
    const auto& destination = frame.getDestinationAddress();
    const auto& source = frame.getSourceAddress();
    bool hasDestination = !destination.isNone(), hasSource = !source.isNone();
    if (frame.getPanCompression() && (!hasDestination || !hasSource || frame.getDestinationPanId() != frame.getSourcePanId()))
        throw cRuntimeError("Invalid legacy IEEE 802.15.4 PAN compression");
    if (frame.getSecurityEnabled() && frame.getComputeFcs())
        throw cRuntimeError("Security transformation is unavailable for generated IEEE 802.15.4 frames");
    if (frame.getFrameType() == 3 && !frame.getPayloadArraySize())
        throw cRuntimeError("IEEE 802.15.4 command needs a Command ID");
    if (frame.getFrameType() == 0 && (hasDestination || !hasSource || frame.getAckRequest() || frame.getPanCompression() ||
            (!frame.getSecurityEnabled() && frame.getPayloadArraySize() < 4)))
        throw cRuntimeError("Invalid legacy IEEE 802.15.4 beacon fields");
    if (frame.getFrameType() == 2 && (hasDestination || hasSource || frame.getAckRequest() || frame.getSecurityEnabled() || frame.getPanCompression() || frame.getPayloadArraySize()))
        throw cRuntimeError("Invalid immediate ACK fields");
    uint16_t control = frame.getFrameType() | (frame.getFramePending() ? 0x10 : 0) |
            (frame.getSecurityEnabled() ? 8 : 0) | (frame.getReservedBit7() ? 0x80 : 0) |
            (frame.getAckRequest() ? 0x20 : 0) | (frame.getPanCompression() ? 0x40 : 0) |
            (addressMode(destination) << 10) | (frame.getFrameVersion() << 12) | (addressMode(source) << 14);
    std::vector<uint8_t> bytes;
    appendLe(bytes, control, 2);
    bytes.push_back(frame.getDsn());
    if (hasDestination) {
        appendLe(bytes, frame.getDestinationPanId(), 2);
        appendAddress(bytes, destination);
    }
    if (hasSource) {
        if (!frame.getPanCompression())
            appendLe(bytes, frame.getSourcePanId(), 2);
        appendAddress(bytes, source);
    }
    for (size_t i = 0; i < frame.getPayloadArraySize(); ++i)
        bytes.push_back(frame.getPayload(i));
    if (frame.getFcsPresent())
        appendLe(bytes, frame.getComputeFcs() ? calculateFcs(bytes) : frame.getFcs(), 2);
    if (bytes.size() + (frame.getFcsPresent() ? 0 : 2) > 127)
        throw cRuntimeError("IEEE 802.15.4 O-QPSK PSDU exceeds 127 octets");
    return bytes;
}

void Ieee802154FrameCodec::finalize(Ieee802154Frame& frame)
{
    frame.setChunkLength(B(encode(frame).size()));
}

Ptr<Ieee802154Frame> Ieee802154FrameCodec::decode(const std::vector<uint8_t>& bytes, bool fcsPresent)
{
    auto frame = makeShared<Ieee802154Frame>();
    frame->setFcsPresent(fcsPresent);
    frame->setComputeFcs(false);
    frame->setChunkLength(B(bytes.size()));
    auto failure = [&](DecodeStatus status) {
        frame->setDecodeStatus(status);
        frame->setRawBytesArraySize(bytes.size());
        for (size_t i = 0; i < bytes.size(); ++i)
            frame->setRawBytes(i, bytes[i]);
        if (status == TRUNCATED)
            frame->markIncomplete();
        else
            frame->markImproperlyRepresented();
        return frame;
    };
    if (bytes.size() < size_t(fcsPresent ? 5 : 3))
        return failure(TRUNCATED);
    if (bytes.size() + (fcsPresent ? 0 : 2) > 127)
        return failure(MALFORMED);
    size_t limit = bytes.size() - (fcsPresent ? 2 : 0);
    uint16_t control = bytes[0] | uint16_t(bytes[1]) << 8;
    int type = control & 7, destinationMode = (control >> 10) & 3;
    int version = (control >> 12) & 3, sourceMode = (control >> 14) & 3;
    if (version == 3 || destinationMode == 1 || sourceMode == 1 || type == 4)
        return failure(MALFORMED);
    if (version == 2 || type > 3)
        return failure(UNSUPPORTED);
    if (type != 0 && (control & 0x300))
        return failure(MALFORMED); // 7.2.2: sequence suppression and IE bits in legacy frames.
    bool compressed = type != 0 && (control & 0x40); // Legacy beacon reserved FCF fields are ignored.
    if (compressed && (!destinationMode || !sourceMode))
        return failure(MALFORMED);
    if (type == 2 && (destinationMode || sourceMode || (control & 0x68) || limit != 3))
        return failure(MALFORMED);
    if (type == 0 && (destinationMode || !sourceMode))
        return failure(MALFORMED);
    frame->setFrameType(type);
    frame->setFrameVersion(version);
    frame->setAckRequest(type != 0 && (control & 0x20));
    frame->setSecurityEnabled(control & 8);
    frame->setReservedBit7(control & 0x80); // 4.6: ignore reserved fields; retain captured bits.
    frame->setFramePending(control & 0x10);
    frame->setPanCompression(compressed);
    frame->setDsn(bytes[2]);
    size_t position = 3;
    auto readLe = [&](unsigned int count) {
        uint64_t value = 0;
        for (unsigned int i = 0; i < count; ++i)
            value |= uint64_t(bytes[position++]) << (i * 8);
        return value;
    };
    unsigned int destinationLength = destinationMode == 2 ? 2 : destinationMode == 3 ? 8 : 0;
    unsigned int sourceLength = sourceMode == 2 ? 2 : sourceMode == 3 ? 8 : 0;
    size_t headerLength = 3 + (destinationLength ? 2 + destinationLength : 0) + (sourceLength ? sourceLength + (compressed ? 0 : 2) : 0);
    if (limit < headerLength)
        return failure(TRUNCATED);
    if (type == 3 && limit == headerLength)
        return failure(TRUNCATED);
    if (type == 0 && !(control & 8) && limit < headerLength + 4)
        return failure(TRUNCATED);
    if (destinationLength) {
        frame->setDestinationPanId(readLe(2));
        auto value = readLe(destinationLength);
        frame->setDestinationAddress(destinationMode == 2 ? Ieee802154Address::fromShort(value) : Ieee802154Address::fromExtended(value));
    }
    if (sourceLength) {
        frame->setSourcePanId(compressed ? frame->getDestinationPanId() : readLe(2));
        auto value = readLe(sourceLength);
        frame->setSourceAddress(sourceMode == 2 ? Ieee802154Address::fromShort(value) : Ieee802154Address::fromExtended(value));
    }
    frame->setPayloadArraySize(limit - position);
    for (size_t i = 0; position < limit; ++i)
        frame->setPayload(i, bytes[position++]);
    if (fcsPresent) {
        auto fcs = uint16_t(bytes[limit]) | uint16_t(bytes[limit + 1]) << 8;
        frame->setFcs(fcs);
        bool valid = fcs == calculateFcs(std::vector<uint8_t>(bytes.begin(), bytes.begin() + limit));
        frame->setFcsValid(valid);
        if (!valid)
            frame->markIncorrect();
    }
    return frame;
}

} // namespace inet
