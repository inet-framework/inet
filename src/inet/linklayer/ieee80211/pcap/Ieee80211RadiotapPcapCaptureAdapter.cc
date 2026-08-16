//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/pcap/Ieee80211RadiotapPcapCaptureAdapter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "inet/common/INETMath.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/recorder/PcapCaptureAdapterRegistry.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INarrowbandSignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"

namespace inet {

namespace {

// Field layouts are defined by the official radiotap specifications:
// https://www.radiotap.org/fields/Flags.html
// https://www.radiotap.org/fields/MCS.html
// https://www.radiotap.org/fields/A-MPDU%20status.html
// https://www.radiotap.org/fields/VHT.html
enum RadiotapPresentBit {
    RADIOTAP_FLAGS = 1,
    RADIOTAP_RATE = 2,
    RADIOTAP_CHANNEL = 3,
    RADIOTAP_ANTENNA_SIGNAL = 5,
    RADIOTAP_DBM_TX_POWER = 10,
    RADIOTAP_RX_FLAGS = 14,
    RADIOTAP_TX_FLAGS = 15,
    RADIOTAP_MCS = 19,
    RADIOTAP_AMPDU = 20,
    RADIOTAP_VHT = 21,
};

enum RadiotapFlags {
    RADIOTAP_F_FCS = 0x10,
    RADIOTAP_F_BADFCS = 0x40,
};

enum RadiotapChannelFlags {
    RADIOTAP_CHANNEL_2GHZ = 0x0080,
    RADIOTAP_CHANNEL_5GHZ = 0x0100,
};

enum RadiotapVhtKnown {
    RADIOTAP_VHT_GI_KNOWN = 1U << 2,
    RADIOTAP_VHT_BANDWIDTH_KNOWN = 1U << 6,
};

struct MpduRange
{
    b offset;
    b length;
};

enum class AmpduParseResult {
    NOT_AGGREGATE,
    VALID,
    INVALID,
};

struct RadiotapRecordMetadata
{
    bool isAmpdu = false;
    bool isLastSubframe = false;
    uint32_t ampduReference = 0;
    bool hasFcs = false;
    bool hasBadFcs = false;
};

struct RadiotapPpduFields
{
    Direction direction = DIRECTION_UNDEFINED;
    bool hasRate = false;
    uint8_t rate = 0;
    bool hasChannel = false;
    uint16_t channelFrequency = 0;
    uint16_t channelFlags = 0;
    bool hasPower = false;
    int8_t power = 0;
    bool isHt = false;
    std::array<uint8_t, 3> mcs = {};
    bool isVht = false;
    uint16_t vhtKnown = 0;
    uint8_t vhtFlags = 0;
    uint8_t vhtBandwidth = 0;
    std::array<uint8_t, 4> vhtMcsNss = {};
    uint8_t vhtCoding = 0;
    uint8_t vhtGroupId = 0;
    uint16_t vhtPartialAid = 0;
};

void appendPadding(std::vector<uint8_t>& bytes, size_t alignment)
{
    bytes.resize(bytes.size() + (alignment - bytes.size() % alignment) % alignment, 0);
}

void appendUint16(std::vector<uint8_t>& bytes, uint16_t value)
{
    bytes.push_back(value & 0xff);
    bytes.push_back(value >> 8);
}

void appendUint32(std::vector<uint8_t>& bytes, uint32_t value)
{
    for (int i = 0; i < 4; i++)
        bytes.push_back((value >> (8 * i)) & 0xff);
}

void setUint16(std::vector<uint8_t>& bytes, size_t offset, uint16_t value)
{
    bytes.at(offset) = value & 0xff;
    bytes.at(offset + 1) = value >> 8;
}

void setUint32(std::vector<uint8_t>& bytes, size_t offset, uint32_t value)
{
    for (size_t i = 0; i < 4; i++)
        bytes.at(offset + i) = value >> (8 * i);
}

uint8_t getRadiotapVhtBandwidth(Hz bandwidth)
{
    auto value = bandwidth.get();
    if (value < 30e6)
        return 0;
    if (value < 60e6)
        return 1;
    if (value < 100e6)
        return 4;
    if (value < 180e6)
        return 11;
    throw cRuntimeError("Unsupported VHT radiotap channel width: %g Hz", value);
}

uint32_t makeAmpduReference(const Packet *packet)
{
    auto treeId = static_cast<uint64_t>(packet->getTreeId());
    return static_cast<uint32_t>(treeId) ^ static_cast<uint32_t>(treeId >> 32);
}

AmpduParseResult getIeee80211AmpduMpduRanges(const Packet *packet, b frontOffset, b backOffset, std::vector<MpduRange>& mpduRanges)
{
    // IEEE 802.11-2024, 9.7.1, Figures 9-1326, 9-1328, 9-1329 and Table 9-659.
    const int parsingFlags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    auto endOffset = packet->getDataLength() - backOffset;
    if (frontOffset + ieee80211::LENGTH_A_MPDU_SUBFRAME_HEADER > endOffset)
        return AmpduParseResult::NOT_AGGREGATE;

    auto peekDelimiter = [&] (b offset) {
        return dynamicPtrCast<const ieee80211::Ieee80211MpduSubframeHeader>(packet->peekDataAt(offset, b(-1), parsingFlags));
    };

    try {
        if (peekDelimiter(frontOffset) == nullptr)
            return AmpduParseResult::NOT_AGGREGATE;
        auto offset = frontOffset;
        while (offset < endOffset) {
            if (offset + ieee80211::LENGTH_A_MPDU_SUBFRAME_HEADER > endOffset)
                return AmpduParseResult::INVALID;
            const auto& delimiter = peekDelimiter(offset);
            if (delimiter == nullptr || delimiter->getLength() < 0)
                return AmpduParseResult::INVALID;
            auto mpduOffset = offset + delimiter->getChunkLength();
            auto mpduLength = B(delimiter->getLength());
            // A zero-length delimiter is representable as VHT EOF/padding and
            // does not itself produce a captured MPDU record.
            if (mpduLength == b(0)) {
                offset = mpduOffset;
                continue;
            }
            if (mpduOffset + mpduLength > endOffset)
                return AmpduParseResult::INVALID;
            mpduRanges.push_back({mpduOffset, mpduLength});
            offset = mpduOffset + mpduLength;
            if (offset == endOffset)
                return AmpduParseResult::VALID;
            auto paddingLength = B((4 - (delimiter->getChunkLength() + mpduLength).get<B>() % 4) % 4);
            // IEEE 802.11-2024, 9.7.1 and 10.12.6 permit exact final-subframe alignment padding for VHT/HE-family PPDUs.
            // Without PHY-mode provenance, accept the structurally complete equality case instead of discarding its MPDUs.
            if (offset + paddingLength > endOffset)
                return AmpduParseResult::INVALID;
            offset += paddingLength;
        }
    }
    catch (cRuntimeError&) {
        return AmpduParseResult::INVALID;
    }
    return AmpduParseResult::VALID;
}

struct FcsMetadata
{
    bool isPresent = false;
    bool isBad = false;
};

FcsMetadata getIeee80211FcsMetadata(const Packet *packet, b frontOffset, b backOffset)
{
    auto endOffset = packet->getDataLength() - backOffset;
    if (endOffset - frontOffset < B(4))
        return {};
    try {
        auto trailer = dynamicPtrCast<const ieee80211::Ieee80211MacTrailer>(packet->peekDataAt(endOffset - B(4), B(4)));
        if (trailer == nullptr)
            return {};
        FcsMetadata metadata;
        metadata.isPresent = true;
        switch (trailer->getFcsMode()) {
            case FCS_DECLARED_INCORRECT:
                metadata.isBad = true;
                break;
            case FCS_COMPUTED: {
                auto data = packet->peekDataAt<BytesChunk>(frontOffset, endOffset - frontOffset - trailer->getChunkLength());
                metadata.isBad = ethernetFcs(data->getBytes()) != trailer->getFcs();
                break;
            }
            case FCS_DECLARED_CORRECT:
            default:
                break;
        }
        return metadata;
    }
    catch (cRuntimeError&) {
        return {};
    }
}

const physicallayer::IIeee80211Mode *findIeee80211Mode(const Packet *packet, const physicallayer::ITransmission *transmission)
{
    if (auto ieee80211Transmission = dynamic_cast<const physicallayer::Ieee80211Transmission *>(transmission)) {
        if (auto mode = ieee80211Transmission->getMode())
            return mode;
    }
    if (auto modeReq = packet->findTag<physicallayer::Ieee80211ModeReq>())
        return modeReq->getMode();
    if (auto modeInd = packet->findTag<physicallayer::Ieee80211ModeInd>())
        return modeInd->getMode();
    return nullptr;
}

RadiotapPpduFields extractRadiotapPpduFields(const Packet *packet, Direction direction, const physicallayer::ITransmission *transmission,
        const physicallayer::IReception *reception)
{
    RadiotapPpduFields fields;
    fields.direction = direction;

    auto mode = findIeee80211Mode(packet, transmission);
    if (mode != nullptr) {
        auto dataMode = mode->getDataMode();
        if (dynamic_cast<const physicallayer::Ieee80211HtMode *>(mode) != nullptr) {
            fields.isHt = true;
            if (auto htDataMode = dynamic_cast<const physicallayer::Ieee80211HtDataMode *>(dataMode)) {
                // IEEE 802.11-2024, Table 19-11; radiotap MCS known/flags/mcs fields.
                fields.mcs[0] = 0x01 | 0x02 | 0x04 | 0x10; // bandwidth, MCS, GI, and BCC FEC are known
                if (htDataMode->getBandwidth().get() > 30e6)
                    fields.mcs[1] |= 1;
                if (htDataMode->getGuardIntervalType() == physicallayer::Ieee80211HtModeBase::HT_GUARD_INTERVAL_SHORT)
                    fields.mcs[1] |= 1 << 2;
                fields.mcs[2] = htDataMode->getMcsIndex();
            }
        }
        else if (dynamic_cast<const physicallayer::Ieee80211VhtMode *>(mode) != nullptr) {
            fields.isVht = true;
            if (auto vhtDataMode = dynamic_cast<const physicallayer::Ieee80211VhtDataMode *>(dataMode)) {
                // IEEE 802.11-2024, Table 21-12; radiotap VHT known/flags fields.
                fields.vhtKnown = RADIOTAP_VHT_GI_KNOWN | RADIOTAP_VHT_BANDWIDTH_KNOWN;
                if (vhtDataMode->getGuardIntervalType() == physicallayer::Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT)
                    fields.vhtFlags |= 0x04;
                fields.vhtBandwidth = getRadiotapVhtBandwidth(vhtDataMode->getBandwidth());
                auto mcs = vhtDataMode->getMcsIndex();
                auto numberOfSpatialStreams = vhtDataMode->getNumberOfSpatialStreams();
                if (mcs <= 9 && numberOfSpatialStreams >= 1 && numberOfSpatialStreams <= 8)
                    fields.vhtMcsNss[0] = (mcs << 4) | numberOfSpatialStreams;
                fields.vhtCoding = 0; // BCC
            }
        }
        else if (dataMode != nullptr) {
            double rateValue = dataMode->getNetBitrate().get() / 500000.0;
            if (std::isfinite(rateValue) && rateValue >= 1 && rateValue <= 255 && rateValue == std::trunc(rateValue)) {
                fields.hasRate = true;
                fields.rate = static_cast<uint8_t>(rateValue);
            }
        }
    }

    const physicallayer::ISignalAnalogModel *analogModel = nullptr;
    simtime_t startTime;
    simtime_t endTime;
    if (reception != nullptr) {
        analogModel = reception->getAnalogModel();
        startTime = reception->getStartTime();
        endTime = reception->getEndTime();
    }
    else if (transmission != nullptr) {
        analogModel = transmission->getAnalogModel();
        startTime = transmission->getStartTime();
        endTime = transmission->getEndTime();
    }

    auto narrowbandAnalogModel = dynamic_cast<const physicallayer::INarrowbandSignalAnalogModel *>(analogModel);
    if (narrowbandAnalogModel != nullptr) {
        double frequencyMHz = narrowbandAnalogModel->getCenterFrequency().get() / 1e6;
        if (std::isfinite(frequencyMHz) && frequencyMHz > 0 && frequencyMHz <= UINT16_MAX) {
            fields.hasChannel = true;
            fields.channelFrequency = static_cast<uint16_t>(std::round(frequencyMHz));
            fields.channelFlags = frequencyMHz < 3000 ? RADIOTAP_CHANNEL_2GHZ : frequencyMHz < 6000 ? RADIOTAP_CHANNEL_5GHZ : 0;
        }

        auto power = narrowbandAnalogModel->computeMinPower(startTime, endTime);
        double powerMilliwatts = power.get<units::values::mW>();
        if (std::isfinite(powerMilliwatts) && powerMilliwatts > 0 &&
                (direction == DIRECTION_INBOUND || direction == DIRECTION_OUTBOUND)) {
            int powerDbm = static_cast<int>(std::round(math::mW2dBmW(powerMilliwatts)));
            fields.hasPower = true;
            fields.power = static_cast<int8_t>(std::clamp(powerDbm, -128, 127));
        }
    }
    return fields;
}

std::vector<uint8_t> serializeRadiotapHeader(const RadiotapPpduFields& fields, const RadiotapRecordMetadata& metadata)
{
    uint32_t present = 0;
    auto setPresentBit = [&] (RadiotapPresentBit bit) { present |= 1U << bit; };
    std::vector<uint8_t> bytes(8, 0);

    setPresentBit(RADIOTAP_FLAGS);
    bytes.push_back((metadata.hasFcs ? RADIOTAP_F_FCS : 0) | (metadata.hasBadFcs ? RADIOTAP_F_BADFCS : 0));
    if (fields.hasRate) {
        setPresentBit(RADIOTAP_RATE);
        bytes.push_back(fields.rate);
    }
    if (fields.hasChannel) {
        setPresentBit(RADIOTAP_CHANNEL);
        appendPadding(bytes, 2);
        appendUint16(bytes, fields.channelFrequency);
        appendUint16(bytes, fields.channelFlags);
    }
    if (fields.hasPower) {
        setPresentBit(fields.direction == DIRECTION_INBOUND ? RADIOTAP_ANTENNA_SIGNAL : RADIOTAP_DBM_TX_POWER);
        bytes.push_back(static_cast<uint8_t>(fields.power));
    }
    if (fields.direction == DIRECTION_INBOUND) {
        setPresentBit(RADIOTAP_RX_FLAGS);
        appendPadding(bytes, 2);
        appendUint16(bytes, 0);
    }
    else if (fields.direction == DIRECTION_OUTBOUND) {
        setPresentBit(RADIOTAP_TX_FLAGS);
        appendPadding(bytes, 2);
        appendUint16(bytes, 0);
    }
    if (fields.isHt) {
        setPresentBit(RADIOTAP_MCS);
        bytes.insert(bytes.end(), fields.mcs.begin(), fields.mcs.end());
    }
    if (metadata.isAmpdu) {
        setPresentBit(RADIOTAP_AMPDU);
        appendPadding(bytes, 4);
        appendUint32(bytes, metadata.ampduReference);
        // Radiotap A-MPDU status: LAST_KNOWN and, for the terminal MPDU, IS_LAST.
        // Delimiter CRC and EOF are intentionally left unknown.
        appendUint16(bytes, 0x0004 | (metadata.isLastSubframe ? 0x0008 : 0));
        bytes.push_back(0);
        bytes.push_back(0);
    }
    if (fields.isVht) {
        setPresentBit(RADIOTAP_VHT);
        appendPadding(bytes, 2);
        appendUint16(bytes, fields.vhtKnown);
        bytes.push_back(fields.vhtFlags);
        bytes.push_back(fields.vhtBandwidth);
        bytes.insert(bytes.end(), fields.vhtMcsNss.begin(), fields.vhtMcsNss.end());
        bytes.push_back(fields.vhtCoding);
        bytes.push_back(fields.vhtGroupId);
        appendUint16(bytes, fields.vhtPartialAid);
    }
    setUint16(bytes, 2, bytes.size());
    setUint32(bytes, 4, present);
    return bytes;
}

} // namespace

namespace ieee80211 {

Register_Pcap_Capture_Adapter(&Protocol::ieee80211Mac, Ieee80211RadiotapPcapCaptureAdapter);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211FhssPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211IrPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211DsssPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211HrDsssPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211OfdmPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211ErpOfdmPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211HtPhy, &Protocol::ieee80211Mac);
Register_Pcap_Capture_Protocol_Resolver(&Protocol::ieee80211VhtPhy, &Protocol::ieee80211Mac);

std::optional<std::pair<b, b>> Ieee80211RadiotapPcapCaptureAdapter::tryResolvePacket(const Packet *packet, b frontOffset, b backOffset) const
{
    const int parsingFlags = Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    try {
        const auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        Ptr<const physicallayer::Ieee80211PhyHeader> header;
        if (*protocol == Protocol::ieee80211FhssPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211FhssPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211IrPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211IrPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211DsssPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211DsssPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211HrDsssPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211HrDsssPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211OfdmPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211OfdmPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211ErpOfdmPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211ErpOfdmPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211HtPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211HtPhyHeader>(frontOffset, b(-1), parsingFlags);
        else if (*protocol == Protocol::ieee80211VhtPhy)
            header = packet->peekDataAt<physicallayer::Ieee80211VhtPhyHeader>(frontOffset, b(-1), parsingFlags);
        else
            return std::nullopt;
        if (header->isIncorrect() || header->isIncomplete() || header->isImproperlyRepresented() ||
                b(header->getLengthField()) <= b(0))
            return std::nullopt;
        auto resolvedFrontOffset = frontOffset + header->getChunkLength();
        auto payloadLength = b(header->getLengthField());
        auto availablePayloadLength = packet->getDataLength() - resolvedFrontOffset - backOffset;
        if (payloadLength > availablePayloadLength)
            return std::nullopt;
        auto resolvedBackOffset = packet->getDataLength() - resolvedFrontOffset - payloadLength;
        return std::pair<b, b>(resolvedFrontOffset, resolvedBackOffset);
    }
    catch (cRuntimeError&) {
        return std::nullopt;
    }
}

std::vector<PcapCaptureRecord> Ieee80211RadiotapPcapCaptureAdapter::createRecords(const PcapCaptureObservation& observation, b frontOffset, b backOffset) const
{
    auto packet = observation.packet;
    auto transmission = dynamic_cast<const physicallayer::ITransmission *>(observation.transmission);
    auto reception = dynamic_cast<const physicallayer::IReception *>(observation.reception);
    const auto ppduFields = extractRadiotapPpduFields(packet, observation.direction, transmission, reception);

    std::vector<MpduRange> mpduRanges;
    auto ampduParseResult = getIeee80211AmpduMpduRanges(packet, frontOffset, backOffset, mpduRanges);
    if (ampduParseResult == AmpduParseResult::INVALID)
        return {};
    if (ampduParseResult == AmpduParseResult::VALID) {
        std::vector<PcapCaptureRecord> records;
        records.reserve(mpduRanges.size());
        auto ampduReference = makeAmpduReference(packet);
        for (size_t i = 0; i < mpduRanges.size(); i++) {
            const auto& mpduRange = mpduRanges[i];
            auto recordBackOffset = packet->getDataLength() - mpduRange.offset - mpduRange.length;
            RadiotapRecordMetadata metadata;
            // Radiotap defines A-MPDU status for received frames. Outbound
            // aggregates are still split, but carry no A-MPDU status field.
            metadata.isAmpdu = observation.direction == DIRECTION_INBOUND;
            metadata.isLastSubframe = i == mpduRanges.size() - 1;
            metadata.ampduReference = ampduReference;
            auto fcsMetadata = getIeee80211FcsMetadata(packet, mpduRange.offset, recordBackOffset);
            metadata.hasFcs = fcsMetadata.isPresent;
            metadata.hasBadFcs = fcsMetadata.isBad;
            records.emplace_back(mpduRange.offset, recordBackOffset, serializeRadiotapHeader(ppduFields, metadata));
        }
        return records;
    }

    RadiotapRecordMetadata metadata;
    auto fcsMetadata = getIeee80211FcsMetadata(packet, frontOffset, backOffset);
    metadata.hasFcs = fcsMetadata.isPresent;
    metadata.hasBadFcs = fcsMetadata.isBad;
    return {PcapCaptureRecord(frontOffset, backOffset, serializeRadiotapHeader(ppduFields, metadata))};
}

} // namespace ieee80211
} // namespace inet
