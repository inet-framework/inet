//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

namespace physicallayer {

void ErrorModelBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *corruptionModeString = par("corruptionMode");
        if (!strcmp("packet", corruptionModeString))
            corruptionMode = CorruptionMode::CM_PACKET;
        else if (!strcmp("chunk", corruptionModeString))
            corruptionMode = CorruptionMode::CM_CHUNK;
        else if (!strcmp("byte", corruptionModeString))
            corruptionMode = CorruptionMode::CM_BYTE;
        else if (!strcmp("bit", corruptionModeString))
            corruptionMode = CorruptionMode::CM_BIT;
        else
            throw cRuntimeError("Unknown corruption mode");
        const char *snirModeString = par("snirMode");
        if (!strcmp("min", snirModeString))
            snirMode = SnirMode::SM_MIN;
        else if (!strcmp("mean", snirModeString))
            snirMode = SnirMode::SM_MEAN;
        else
            throw cRuntimeError("Unknown SNIR mode: '%s'", snirModeString);
        snirOffset = inet::math::dB2fraction(par("snirOffset"));
    }
}

double ErrorModelBase::getScalarSnir(const ISnir *snir) const
{
    double scalarSnir;
    if (snirMode == SnirMode::SM_MIN)
        scalarSnir = snir->getMin();
    else if (snirMode == SnirMode::SM_MEAN)
        scalarSnir = snir->getMean();
    else
        throw cRuntimeError("Unknown SNIR mode");
    return scalarSnir * snirOffset;
}

double ErrorModelBase::getScalarSnir(const ISnir *snir, simtime_t startTime, simtime_t endTime) const
{
    double scalarSnir;
    if (snirMode == SnirMode::SM_MIN)
        scalarSnir = snir->getMin(startTime, endTime);
    else if (snirMode == SnirMode::SM_MEAN)
        scalarSnir = snir->getMean(startTime, endTime);
    else
        throw cRuntimeError("Unknown SNIR mode");
    return scalarSnir * snirOffset;
}

void ErrorModelBase::checkRegionPart(IRadioSignal::SignalPart part) const
{
    if (part != IRadioSignal::SIGNAL_PART_HEADER && part != IRadioSignal::SIGNAL_PART_DATA)
        throw cRuntimeError("Only the header part and the data part have regions, not the %s part", IRadioSignal::getSignalPartName(part));
}

double ErrorModelBase::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const
{
    Enter_Method("computePacketErrorRate");
    // the bits of a region have independent errors with the bit error rate of the region; this form
    // gives exactly 0 for a very small bit error rate, as the rates of the whole signal do
    double bitErrorRate = computeBitErrorRate(snir, part, length, startTime, endTime);
    return 1.0 - std::pow(1.0 - bitErrorRate, length.get<b>());
}

Ptr<const Chunk> ErrorModelBase::computeCorruptedChunk(const Packet *packet, const ErrorRegion& region) const
{
    int flags = Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    switch (corruptionMode) {
        case CorruptionMode::CM_CHUNK: {
            // the received region is an incorrect copy
            const auto& chunk = packet->peekAt(region.offset, region.length, flags);
            if (chunk->isIncorrect())
                return chunk;
            const auto& corruptedChunk = chunk->dupShared();
            corruptedChunk->markIncorrect();
            corruptedChunk->markImmutable();
            return corruptedChunk;
        }
        case CorruptionMode::CM_BYTE:
            // the received region has flipped bytes, or flipped bits if it is not whole bytes
            if (region.length.get<b>() % 8 == 0) {
                auto bytes = packet->peekAt<BytesChunk>(region.offset, region.length, flags)->getBytes();
                // a byte has an error if at least one of its bits has an error
                double byteErrorRate = -std::expm1(8 * std::log1p(-region.bitErrorRate));
                corruptUnits(bytes.size(), byteErrorRate, [&] (int i) { bytes[i] = ~bytes[i]; });
                const auto& corruptedChunk = makeShared<BytesChunk>(bytes);
                corruptedChunk->markIncorrect();
                corruptedChunk->markImmutable();
                return corruptedChunk;
            }
            [[fallthrough]];
        case CorruptionMode::CM_BIT: {
            // the received region has flipped bits
            auto bits = packet->peekAt<BitsChunk>(region.offset, region.length, flags)->getBits();
            corruptUnits(bits.size(), region.bitErrorRate, [&] (int i) { bits[i] = !bits[i]; });
            const auto& corruptedChunk = makeShared<BitsChunk>(bits);
            corruptedChunk->markIncorrect();
            corruptedChunk->markImmutable();
            return corruptedChunk;
        }
        default:
            throw cRuntimeError("The corruption mode has no corrupted regions");
    }
}

void ErrorModelBase::corruptUnits(int numUnits, double unitErrorRate, const std::function<void(int)>& corruptUnit) const
{
    ASSERT(numUnits > 0 && 0 <= unitErrorRate && unitErrorRate <= 1);
    if (unitErrorRate == 1) {
        for (int i = 0; i < numUnits; i++)
            corruptUnit(i);
    }
    else if (unitErrorRate == 0)
        // the region failed by the SNIR threshold, and its error rate is 0
        corruptUnit(intuniform(0, numUnits - 1));
    else {
        double logUnitSuccessRate = std::log1p(-unitErrorRate);
        // the probability of at least one error in the region, which can round to 1
        double errorRate = -std::expm1(numUnits * logUnitSuccessRate);
        // the first error, under the condition of at least one error in the region
        double position = std::min(std::floor(std::log1p(-dblrand() * errorRate) / logUnitSuccessRate), numUnits - 1.0);
        while (position < numUnits) {
            corruptUnit((int)position);
            // the geometric gap to the next error
            position += 1 + std::floor(std::log1p(-dblrand()) / logUnitSuccessRate);
        }
    }
}

Packet *ErrorModelBase::computeCorruptedPacket(const Packet *packet, const std::vector<ErrorRegion>& regions) const
{
    // the caller owns the received packet, so the function does not enter the module
    auto corruptedPacket = packet->dup();
    // the packet mode marks the whole packet
    if (corruptionMode == CorruptionMode::CM_PACKET) {
        corruptedPacket->setBitError(true);
        return corruptedPacket;
    }
    // the content between the failed regions stays as the transmitter sent it
    int flags = Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED;
    auto content = makeShared<SequenceChunk>();
    b position = b(0);
    for (const auto& region : regions) {
        if (region.offset > position)
            content->insertAtBack(packet->peekAt(position, region.offset - position, flags));
        content->insertAtBack(computeCorruptedChunk(packet, region));
        position = region.offset + region.length;
    }
    b totalLength = packet->getTotalLength();
    if (position < totalLength)
        content->insertAtBack(packet->peekAt(position, totalLength - position, flags));
    corruptedPacket->replaceAll(content, flags);
    return corruptedPacket;
}

} // namespace physicallayer

} // namespace inet

