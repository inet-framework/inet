//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPhyHeaderSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Register_Serializer(EthernetPhyHeaderBase, EthernetPhyHeaderBaseSerializer);
Register_Serializer(EthernetPhyHeader, EthernetPhyHeaderSerializer);
Register_Serializer(EthernetFragmentPhyHeader, EthernetFragmentPhyHeaderSerializer);

void EthernetPhyHeaderBaseSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    throw cRuntimeError("Invalid operation");
}

const Ptr<Chunk> EthernetPhyHeaderBaseSerializer::deserialize(MemoryInputStream& stream) const
{
    uint8_t byte = stream.getData().at(PREAMBLE_BYTES.get<B>());
    if (byte == 0xD5) {
        EthernetPhyHeaderSerializer serializer;
        return serializer.deserialize(stream);
    }
    else {
        EthernetFragmentPhyHeaderSerializer serializer;
        return serializer.deserialize(stream);
    }
}

void EthernetPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    stream.writeByteRepeatedly(0x55, PREAMBLE_BYTES.get<B>()); // preamble
    stream.writeByte(0xD5); // SFD
}

const Ptr<Chunk> EthernetPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<EthernetPhyHeader>();
    bool preambleReadSuccessfully = stream.readByteRepeatedly(0x55, PREAMBLE_BYTES.get<B>()); // preamble
    uint8_t sfd = stream.readByte();
    if (!preambleReadSuccessfully || sfd != 0xD5) {
        header->markIncorrect();
        header->markImproperlyRepresented();
    }
    return header;
}

void EthernetFragmentPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const EthernetFragmentPhyHeader>(chunk);
    stream.writeByteRepeatedly(0x55, PREAMBLE_BYTES.get<B>() - (header->getPreambleType() == SMD_Cx ? 1 : 0));
    switch (header->getPreambleType()) {
        case SFD:
            stream.writeByte(0xD5);
            break;
        case SMD_Verify:
            stream.writeByte(0x07);
            break;
        case SMD_Respond:
            stream.writeByte(0x19);
            break;
        case SMD_Sx: {
            int smdSxValues[] = { 0xE6, 0x4C, 0x7F, 0xB3 };
            if (header->getSmdNumber() >= 4)
                throw cRuntimeError("Cannot serialize Ethernet fragment PHY header: SMD number %d is out of range", header->getSmdNumber());
            stream.writeByte(smdSxValues[header->getSmdNumber()]);
            break;
        }
        case SMD_Cx: {
            int smdCxValues[] = { 0x61, 0x52, 0x9E, 0x2A };
            if (header->getSmdNumber() >= 4 || header->getFragmentNumber() >= 4)
                throw cRuntimeError("Cannot serialize Ethernet fragment PHY header: SMD number %d / fragment number %d is out of range",
                        header->getSmdNumber(), header->getFragmentNumber());
            stream.writeByte(smdCxValues[header->getSmdNumber()]);
            int fragmentNumberValues[] = { 0xE6, 0x4C, 0x7F, 0xB3 };
            stream.writeByte(fragmentNumberValues[header->getFragmentNumber()]);
            break;
        }
    }
}

const Ptr<Chunk> EthernetFragmentPhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto header = makeShared<EthernetFragmentPhyHeader>();
    bool preambleReadSuccessfully = stream.readByteRepeatedly(0x55, PREAMBLE_BYTES.get<B>() - 1);
    if (!preambleReadSuccessfully)
        header->markIncorrect();
    static const int smdSxValues[] = { 0xE6, 0x4C, 0x7F, 0xB3 };
    static const int smdCxValues[] = { 0x61, 0x52, 0x9E, 0x2A };
    static const int fragmentNumberValues[] = { 0xE6, 0x4C, 0x7F, 0xB3 };
    // an SMD/fragment code outside its table has no representation in the model, so the
    // header is marked incorrect instead of storing an out-of-range index that the
    // serializer would then use to index the table
    auto lookup = [&header] (const int *values, uint8_t value) {
        auto it = std::find(values, values + 4, value);
        if (it == values + 4) {
            header->markIncorrect();
            return 0;
        }
        return (int)std::distance(values, it);
    };
    uint8_t value = stream.readByte();
    if (value == 0x55) {
        uint8_t smd = stream.readByte();
        switch (smd) {
            case 0xD5:
                header->setPreambleType(SFD);
                break;
            case 0x07:
                header->setPreambleType(SMD_Verify);
                break;
            case 0x19:
                header->setPreambleType(SMD_Respond);
                break;
            default:
                header->setPreambleType(SMD_Sx);
                header->setSmdNumber(lookup(smdSxValues, smd));
                break;
        }
    }
    else {
        // an SMD_Cx preamble is one octet shorter, so the octet read above is already the
        // SMD_Cx code; the fragment-count octet follows it
        header->setPreambleType(SMD_Cx);
        header->setSmdNumber(lookup(smdCxValues, value));
        header->setFragmentNumber(lookup(fragmentNumberValues, stream.readByte()));
    }
    return header;
}

} // namespace physicallayer

} // namespace inet

