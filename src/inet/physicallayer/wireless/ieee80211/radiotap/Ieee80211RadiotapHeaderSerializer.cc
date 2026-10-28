//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/physicallayer/wireless/ieee80211/radiotap/Ieee80211RadiotapHeader_m.h"

namespace inet {

namespace physicallayer {

Register_Serializer(Ieee80211RadiotapHeader, Ieee80211RadiotapHeaderSerializer);

namespace {

struct FieldLayout {
    int alignment; // in octets, relative to the start of the header
    int size; // in octets, 0 for a bit that has no defined field
};

// The alignment and size of the fields 0 to 27 in the Radiotap namespace, see
// https://www.radiotap.org/fields/defined. Bit 18 has no defined field.
const FieldLayout fieldLayouts[] = {
    {8, 8}, {1, 1}, {1, 1}, {2, 4}, {2, 2}, {1, 1}, {1, 1}, {2, 2},
    {2, 2}, {2, 2}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {2, 2}, {2, 2},
    {1, 1}, {1, 1}, {0, 0}, {1, 3}, {4, 8}, {2, 12}, {8, 12}, {2, 12},
    {2, 12}, {2, 6}, {1, 1}, {2, 4},
};

const int numFieldLayouts = sizeof(fieldLayouts) / sizeof(fieldLayouts[0]);

const uint32_t representedFields = (1u << RADIOTAP_TSFT) | (1u << RADIOTAP_FLAGS) | (1u << RADIOTAP_RATE) |
        (1u << RADIOTAP_CHANNEL) | (1u << RADIOTAP_DBM_ANTSIGNAL) | (1u << RADIOTAP_DBM_ANTNOISE) |
        (1u << RADIOTAP_DBM_TX_POWER) | (1u << RADIOTAP_MCS) | (1u << RADIOTAP_AMPDU_STATUS) | (1u << RADIOTAP_VHT);

// The bits 29 to 31 select another namespace or another present word.
const uint32_t namespaceBits = 0xE0000000u;

int getPadding(int offset, int alignment)
{
    return (alignment - offset % alignment) % alignment;
}

} // namespace

B computeIeee80211RadiotapHeaderLength(uint32_t present)
{
    if (present & ~representedFields)
        throw cRuntimeError("The Radiotap present word 0x%08x has fields that Ieee80211RadiotapHeader does not represent", present);
    int length = 8;
    for (int bit = 0; bit < numFieldLayouts; bit++) {
        if (present & (1u << bit)) {
            length += getPadding(length, fieldLayouts[bit].alignment);
            length += fieldLayouts[bit].size;
        }
    }
    return B(length);
}

void Ieee80211RadiotapHeaderSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const Ieee80211RadiotapHeader>(chunk);
    uint32_t present = header->getPresent();
    auto startPosition = stream.getLength();
    stream.writeByte(0); // it_version
    stream.writeByte(0); // it_pad
    stream.writeUint16Le(computeIeee80211RadiotapHeaderLength(present).get<B>());
    stream.writeUint32Le(present);
    for (int bit = 0; bit < numFieldLayouts; bit++) {
        if (!(present & (1u << bit)))
            continue;
        int offset = B(stream.getLength() - startPosition).get<B>();
        stream.writeByteRepeatedly(0, getPadding(offset, fieldLayouts[bit].alignment));
        switch (bit) {
            case RADIOTAP_TSFT:
                stream.writeUint64Le(header->getTsft());
                break;
            case RADIOTAP_FLAGS:
                stream.writeByte(header->getFlags());
                break;
            case RADIOTAP_RATE:
                stream.writeByte(header->getRate());
                break;
            case RADIOTAP_CHANNEL:
                stream.writeUint16Le(header->getChannelFrequency());
                stream.writeUint16Le(header->getChannelFlags());
                break;
            case RADIOTAP_DBM_ANTSIGNAL:
                stream.writeByte(header->getAntennaSignal());
                break;
            case RADIOTAP_DBM_ANTNOISE:
                stream.writeByte(header->getAntennaNoise());
                break;
            case RADIOTAP_DBM_TX_POWER:
                stream.writeByte(header->getTxPower());
                break;
            case RADIOTAP_MCS:
                stream.writeByte(header->getMcsKnown());
                stream.writeByte(header->getMcsFlags());
                stream.writeByte(header->getMcsIndex());
                break;
            case RADIOTAP_AMPDU_STATUS:
                stream.writeUint32Le(header->getAmpduReference());
                stream.writeUint16Le(header->getAmpduFlags());
                stream.writeByte(header->getAmpduDelimiterCrc());
                stream.writeByte(0); // reserved
                break;
            case RADIOTAP_VHT:
                stream.writeUint16Le(header->getVhtKnown());
                stream.writeByte(header->getVhtFlags());
                stream.writeByte(header->getVhtBandwidth());
                for (int user = 0; user < 4; user++)
                    stream.writeByte(header->getVhtMcsNss(user));
                stream.writeByte(header->getVhtCoding());
                stream.writeByte(header->getVhtGroupId());
                stream.writeUint16Le(header->getVhtPartialAid());
                break;
        }
    }
}

const Ptr<Chunk> Ieee80211RadiotapHeaderSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    auto header = makeShared<Ieee80211RadiotapHeader>();
    auto startPosition = stream.getPosition();
    uint8_t version = stream.readByte();
    stream.readByte(); // it_pad
    uint16_t length = stream.readUint16Le();
    uint32_t present = stream.readUint32Le();
    header->setPresent(present);
    bool isRepresented = (present & namespaceBits) == 0;
    // the fields of the first present word come after all present words
    for (uint32_t word = present; word & (1u << 31) && !stream.isReadBeyondEnd();) {
        word = stream.readUint32Le();
        isRepresented = false;
    }
    for (int bit = 0; bit < 29; bit++) {
        if (!(present & (1u << bit)))
            continue;
        if (bit >= numFieldLayouts || fieldLayouts[bit].size == 0) {
            // the size of this field is unknown, so the later fields cannot be found
            isRepresented = false;
            break;
        }
        int offset = B(stream.getPosition() - startPosition).get<B>();
        stream.readByteRepeatedly(0, getPadding(offset, fieldLayouts[bit].alignment));
        switch (bit) {
            case RADIOTAP_TSFT:
                header->setTsft(stream.readUint64Le());
                break;
            case RADIOTAP_FLAGS:
                header->setFlags(stream.readByte());
                break;
            case RADIOTAP_RATE:
                header->setRate(stream.readByte());
                break;
            case RADIOTAP_CHANNEL:
                header->setChannelFrequency(stream.readUint16Le());
                header->setChannelFlags(stream.readUint16Le());
                break;
            case RADIOTAP_DBM_ANTSIGNAL:
                header->setAntennaSignal(stream.readByte());
                break;
            case RADIOTAP_DBM_ANTNOISE:
                header->setAntennaNoise(stream.readByte());
                break;
            case RADIOTAP_DBM_TX_POWER:
                header->setTxPower(stream.readByte());
                break;
            case RADIOTAP_MCS:
                header->setMcsKnown(stream.readByte());
                header->setMcsFlags(stream.readByte());
                header->setMcsIndex(stream.readByte());
                break;
            case RADIOTAP_AMPDU_STATUS:
                header->setAmpduReference(stream.readUint32Le());
                header->setAmpduFlags(stream.readUint16Le());
                header->setAmpduDelimiterCrc(stream.readByte());
                stream.readByte(); // reserved
                break;
            case RADIOTAP_VHT:
                header->setVhtKnown(stream.readUint16Le());
                header->setVhtFlags(stream.readByte());
                header->setVhtBandwidth(stream.readByte());
                for (int user = 0; user < 4; user++)
                    header->setVhtMcsNss(user, stream.readByte());
                header->setVhtCoding(stream.readByte());
                header->setVhtGroupId(stream.readByte());
                header->setVhtPartialAid(stream.readUint16Le());
                break;
            default:
                // the size of this field is known, but the chunk has no place for its value
                stream.readByteRepeatedly(0, fieldLayouts[bit].size);
                isRepresented = false;
                break;
        }
    }
    auto endPosition = startPosition + B(length);
    if (version != 0 || stream.getPosition() > endPosition)
        header->markIncorrect();
    else if (endPosition > stream.getLength()) {
        stream.seek(stream.getLength());
        header->markIncomplete();
    }
    else
        stream.seek(endPosition);
    if (!isRepresented)
        header->markImproperlyRepresented();
    return header;
}

} // namespace physicallayer

} // namespace inet
