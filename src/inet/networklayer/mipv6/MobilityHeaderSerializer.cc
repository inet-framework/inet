//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mipv6/MobilityHeaderSerializer.h"

#include <algorithm>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"

namespace inet {

// RFC 6275 Section 6.1.7/6.1.8: binding lifetimes are carried on the wire in
// units of 4 seconds (16-bit field). The chunk stores the value in seconds;
// this quantization is applied only here, at the wire boundary, and over-long
// lifetimes are clamped to the field maximum (0xFFFF * 4 s) rather than wrapping.
static constexpr int BINDING_LIFETIME_UNIT = 4;

Register_Serializer(MobilityHeader, MobilityHeaderSerializer);
Register_Serializer(BindingUpdate, MobilityHeaderSerializer);
Register_Serializer(BindingAcknowledgement, MobilityHeaderSerializer);
Register_Serializer(BindingError, MobilityHeaderSerializer);
Register_Serializer(HomeTestInit, MobilityHeaderSerializer);
Register_Serializer(HomeTest, MobilityHeaderSerializer);
Register_Serializer(CareOfTestInit, MobilityHeaderSerializer);
Register_Serializer(CareOfTest, MobilityHeaderSerializer);
Register_Serializer(BindingRefreshRequest, MobilityHeaderSerializer);

// RFC 6275 Section 6.1 - Mobility Header
//
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    | Payload Proto |  Header Len   |   MH Type     |   Reserved   |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |           Checksum            |                               |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
//    |                                                               |
//    .                       Message Data                            .
//    .                                                               .
//    |                                                               |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// The common MH header is 6 bytes. The Header Len field is in units of
// 8 octets, not including the first 8 octets.

void MobilityHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    b startPos = stream.getLength();
    const auto& mh = staticPtrCast<const MobilityHeader>(chunk);

    // Payload Proto and Header Len will be filled by the IPv6 layer when
    // the MH is used as an extension header. For standalone serialization
    // we write placeholder values.
    stream.writeByte(59); // Payload Proto: No Next Header
    B totalLen = mh->getChunkLength();
    stream.writeByte((totalLen.get() - 8) / 8); // Header Len (in 8-octet units, excluding first 8)
    stream.writeByte(mh->getMobilityHeaderType()); // MH Type
    stream.writeByte(0); // Reserved
    stream.writeUint16Be(0); // Checksum (computed elsewhere)

    switch (mh->getMobilityHeaderType()) {
        case BINDING_REFRESH_REQUEST: {
            // RFC 6275 Section 6.1.2: reserved (2 bytes)
            stream.writeUint16Be(0);
            break;
        }

        case HOME_TEST_INIT: {
            // RFC 6275 Section 6.1.3: reserved (2 bytes) + home init cookie (8 bytes)
            auto hoti = staticPtrCast<const HomeTestInit>(chunk);
            stream.writeUint16Be(0);
            stream.writeUint64Be(hoti->getHomeInitCookie());
            break;
        }

        case CARE_OF_TEST_INIT: {
            // RFC 6275 Section 6.1.4: reserved (2 bytes) + care-of init cookie (8 bytes)
            auto coti = staticPtrCast<const CareOfTestInit>(chunk);
            stream.writeUint16Be(0);
            stream.writeUint64Be(coti->getCareOfInitCookie());
            break;
        }

        case HOME_TEST: {
            // RFC 6275 Section 6.1.5: home nonce index (2 bytes) + home init cookie (8 bytes) + home keygen token (8 bytes)
            auto hot = staticPtrCast<const HomeTest>(chunk);
            stream.writeUint16Be(0); // home nonce index (not modelled)
            stream.writeUint64Be(hot->getHomeInitCookie());
            stream.writeUint64Be(hot->getHomeKeyGenToken());
            break;
        }

        case CARE_OF_TEST: {
            // RFC 6275 Section 6.1.6: care-of nonce index (2 bytes) + care-of init cookie (8 bytes) + care-of keygen token (8 bytes)
            auto cot = staticPtrCast<const CareOfTest>(chunk);
            stream.writeUint16Be(0); // care-of nonce index (not modelled)
            stream.writeUint64Be(cot->getCareOfInitCookie());
            stream.writeUint64Be(cot->getCareOfKeyGenToken());
            break;
        }

        case BINDING_UPDATE: {
            // RFC 6275 Section 6.1.7: sequence (2 bytes) + flags+reserved (2 bytes) + lifetime (2 bytes)
            auto bu = staticPtrCast<const BindingUpdate>(chunk);
            stream.writeUint16Be(bu->getSequence());
            uint16_t flags = (bu->getAckFlag() ? 0x8000u : 0)
                    | (bu->getHomeRegistrationFlag() ? 0x4000u : 0)
                    | (bu->getLinkLocalAddressCompatibilityFlag() ? 0x2000u : 0)
                    | (bu->getKeyManagementFlag() ? 0x1000u : 0);
            stream.writeUint16Be(flags);
            stream.writeUint16Be(std::min(bu->getLifetime() / BINDING_LIFETIME_UNIT, 0xFFFFu));
            // Mobility options: write remaining bytes as padding
            b dataWritten = stream.getLength() - startPos;
            b remaining = b(totalLen) - dataWritten;
            if (remaining > b(0))
                stream.writeByteRepeatedly(0, B(remaining).get());
            break;
        }

        case BINDING_ACKNOWLEDGEMENT: {
            // RFC 6275 Section 6.1.8: status (1 byte) + K flag + reserved (1 byte) + sequence (2 bytes) + lifetime (2 bytes)
            auto ba = staticPtrCast<const BindingAcknowledgement>(chunk);
            stream.writeByte(ba->getStatus());
            stream.writeByte(ba->getKeyManagementFlag() ? 0x80u : 0);
            stream.writeUint16Be(ba->getSequenceNumber());
            stream.writeUint16Be(std::min(ba->getLifetime() / BINDING_LIFETIME_UNIT, 0xFFFFu));
            // Mobility options: write remaining bytes as padding
            b dataWritten = stream.getLength() - startPos;
            b remaining = b(totalLen) - dataWritten;
            if (remaining > b(0))
                stream.writeByteRepeatedly(0, B(remaining).get());
            break;
        }

        case BINDING_ERROR: {
            // RFC 6275 Section 6.1.9: status (1 byte) + reserved (1 byte) + home address (16 bytes)
            auto be = staticPtrCast<const BindingError>(chunk);
            stream.writeByte(be->getStatus());
            stream.writeByte(0);
            stream.writeIpv6Address(be->getHomeAddress());
            break;
        }

        default:
            throw cRuntimeError("Cannot serialize MobilityHeader: type %d not supported.", mh->getMobilityHeaderType());
    }

    // Verify we wrote exactly the right number of bytes
    ASSERT(stream.getLength() - startPos == b(totalLen));
}

const Ptr<Chunk> MobilityHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    b startPos = stream.getPosition();

    uint8_t payloadProto = stream.readByte(); (void)payloadProto;
    uint8_t headerLen = stream.readByte();
    B totalLen = B(8 + headerLen * 8);
    uint8_t mhType = stream.readByte();
    stream.readByte(); // reserved
    stream.readUint16Be(); // checksum

    Ptr<MobilityHeader> mh;

    switch (mhType) {
        case BINDING_REFRESH_REQUEST: {
            auto brr = makeShared<BindingRefreshRequest>();
            brr->setMobilityHeaderType(BINDING_REFRESH_REQUEST);
            stream.readUint16Be(); // reserved
            mh = brr;
            break;
        }

        case HOME_TEST_INIT: {
            auto hoti = makeShared<HomeTestInit>();
            hoti->setMobilityHeaderType(HOME_TEST_INIT);
            stream.readUint16Be(); // reserved
            hoti->setHomeInitCookie(stream.readUint64Be());
            mh = hoti;
            break;
        }

        case CARE_OF_TEST_INIT: {
            auto coti = makeShared<CareOfTestInit>();
            coti->setMobilityHeaderType(CARE_OF_TEST_INIT);
            stream.readUint16Be(); // reserved
            coti->setCareOfInitCookie(stream.readUint64Be());
            mh = coti;
            break;
        }

        case HOME_TEST: {
            auto hot = makeShared<HomeTest>();
            hot->setMobilityHeaderType(HOME_TEST);
            stream.readUint16Be(); // home nonce index (not modelled)
            hot->setHomeInitCookie(stream.readUint64Be());
            hot->setHomeKeyGenToken(stream.readUint64Be());
            mh = hot;
            break;
        }

        case CARE_OF_TEST: {
            auto cot = makeShared<CareOfTest>();
            cot->setMobilityHeaderType(CARE_OF_TEST);
            stream.readUint16Be(); // care-of nonce index (not modelled)
            cot->setCareOfInitCookie(stream.readUint64Be());
            cot->setCareOfKeyGenToken(stream.readUint64Be());
            mh = cot;
            break;
        }

        case BINDING_UPDATE: {
            auto bu = makeShared<BindingUpdate>();
            bu->setMobilityHeaderType(BINDING_UPDATE);
            bu->setSequence(stream.readUint16Be());
            uint16_t flags = stream.readUint16Be();
            bu->setAckFlag((flags & 0x8000u) != 0);
            bu->setHomeRegistrationFlag((flags & 0x4000u) != 0);
            bu->setLinkLocalAddressCompatibilityFlag((flags & 0x2000u) != 0);
            bu->setKeyManagementFlag((flags & 0x1000u) != 0);
            bu->setLifetime(stream.readUint16Be() * BINDING_LIFETIME_UNIT);
            // Skip remaining mobility options
            b consumed = stream.getPosition() - startPos;
            b remaining = b(totalLen) - consumed;
            if (remaining > b(0))
                stream.readByteRepeatedly(0, B(remaining).get());
            mh = bu;
            break;
        }

        case BINDING_ACKNOWLEDGEMENT: {
            auto ba = makeShared<BindingAcknowledgement>();
            ba->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT);
            ba->setStatus(static_cast<BaStatus>(stream.readByte()));
            uint8_t kFlag = stream.readByte();
            ba->setKeyManagementFlag((kFlag & 0x80u) != 0);
            ba->setSequenceNumber(stream.readUint16Be());
            ba->setLifetime(stream.readUint16Be() * BINDING_LIFETIME_UNIT);
            // Skip remaining mobility options
            b consumed = stream.getPosition() - startPos;
            b remaining = b(totalLen) - consumed;
            if (remaining > b(0))
                stream.readByteRepeatedly(0, B(remaining).get());
            mh = ba;
            break;
        }

        case BINDING_ERROR: {
            auto be = makeShared<BindingError>();
            be->setMobilityHeaderType(BINDING_ERROR);
            be->setStatus(static_cast<BeStatus>(stream.readByte()));
            stream.readByte(); // reserved
            be->setHomeAddress(stream.readIpv6Address());
            mh = be;
            break;
        }

        default: {
            EV_ERROR << "Cannot parse MobilityHeader: type " << (int)mhType << " not supported.";
            mh = makeShared<MobilityHeader>();
            mh->markImproperlyRepresented();
            break;
        }
    }

    mh->setChunkLength(totalLen);
    return mh;
}

} // namespace inet
