//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketSourceBase.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/DirectionTag_m.h"
#include "inet/common/IdentityTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag_m.h"

namespace inet {
namespace queueing {

void PacketSourceBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetNameFormat = par("packetNameFormat");
        packetRepresentation = par("packetRepresentation");
        packetLengthParameter = &par("packetLength");
        packetDataParameter = &par("packetData");
        attachCreationTimeTag = par("attachCreationTimeTag");
        attachIdentityTag = par("attachIdentityTag");
        attachDirectionTag = par("attachDirectionTag");
    }
}

const char *PacketSourceBase::createPacketName(const Ptr<const Chunk>& data) const
{
    return StringFormat::formatString(packetNameFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'a': {
                auto application = findContainingApplication();
                if (application != nullptr)
                    result = application->getDisplayName() != nullptr ? application->getDisplayName() : application->getFullName();
                else
                    result = getDisplayName() != nullptr ? getDisplayName() :  getFullName();
                break;
            }
            case 'n':
                result = getDisplayName() != nullptr ? getDisplayName() : getFullName();
                break;
            case 'm': {
                auto application = getContainingApplication();
                result = application->getDisplayName() != nullptr ? application->getDisplayName() : application->getFullName();
                break;
            }
            case 'M': {
                auto networkNode = getContainingNode(this);
                result = networkNode->getDisplayName() != nullptr ? networkNode->getDisplayName() : networkNode->getFullName();
                break;
            }
            case 'p':

                result = getFullPath();
                break;
            case 'h':
                result = getContainingApplication()->getFullPath();
                break;
            case 'H':
                result = getContainingNode(this)->getFullPath();
                break;
            case 'c':
                result = std::to_string(numProcessedPackets);
                break;
            case 'l':
                result = data->getChunkLength().str();
                break;
            case 'd':
                if (auto byteCountChunk = dynamicPtrCast<const ByteCountChunk>(data))
                    result = std::to_string(byteCountChunk->getData());
                else if (auto bitCountChunk = dynamicPtrCast<const BitCountChunk>(data))
                    result = std::to_string(bitCountChunk->getData());
                break;
            case 't':
                result = simTime().str();
                break;
            case 'e':
                result = std::to_string(getSimulation()->getEventNumber());
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
}

Ptr<Chunk> PacketSourceBase::createPacketContent() const
{
    auto packetLength = b(packetLengthParameter->intValue());
    if (!strcmp(packetRepresentation, "bitCount")) {
        int packetData = packetDataParameter->intValue();
        return packetData == -1 ? makeShared<BitCountChunk>(packetLength) : makeShared<BitCountChunk>(packetLength, packetData);
    }
    else if (!strcmp(packetRepresentation, "bits")) {
        static int total = 0;
        const auto& packetContent = makeShared<BitsChunk>();
        std::vector<bool> bits;
        bits.resize(b(packetLength).get());
        for (int i = 0; i < (int)bits.size(); i++) {
            int packetData = packetDataParameter->intValue();
            bits[i] = packetData == -1 ? (total + i) % 2 == 0 : packetData;
        }
        total += bits.size();
        packetContent->setBits(bits);
        return packetContent;
    }
    else if (!strcmp(packetRepresentation, "byteCount")) {
        int packetData = packetDataParameter->intValue();
        return packetData == -1 ? makeShared<ByteCountChunk>(packetLength) : makeShared<ByteCountChunk>(packetLength, packetData);
    }
    else if (!strcmp(packetRepresentation, "bytes")) {
        static int total = 0;
        const auto& packetContent = makeShared<BytesChunk>();
        std::vector<uint8_t> bytes;
        bytes.resize(B(packetLength).get());
        for (int i = 0; i < (int)bytes.size(); i++) {
            int packetData = packetDataParameter->intValue();
            bytes[i] = packetData == -1 ? (total + i) % 256 : packetData;
        }
        total += bytes.size();
        packetContent->setBytes(bytes);
        return packetContent;
    }
    else if (!strcmp(packetRepresentation, "applicationPacket")) {
        const auto& packetContent = makeShared<ApplicationPacket>();
        packetContent->setChunkLength(B(packetLength));
        packetContent->setSequenceNumber(numProcessedPackets);
        return packetContent;
    }
    else
        throw cRuntimeError("Unknown representation");
}

Packet *PacketSourceBase::createPacket()
{
    auto packetContent = createPacketContent();
    if (attachCreationTimeTag)
        packetContent->addTag<CreationTimeTag>()->setCreationTime(simTime());
    if (attachIdentityTag) {
        auto identityStart = IdentityTag::getNextIdentityStart(packetContent->getChunkLength());
        packetContent->addTag<IdentityTag>()->setIdentityStart(identityStart);
    }
    auto packetName = createPacketName(packetContent);
    auto packet = new Packet(packetName, packetContent);
    if (attachDirectionTag)
        packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
    emit(packetCreatedSignal, packet);
    return packet;
}

inline bool isApplication(const cModule *mod)
{
    cProperties *properties = mod->getProperties();
    return properties && properties->getAsBool("application");
}

const cModule *PacketSourceBase::findContainingApplication() const
{
    for (const cModule *module = this; module != nullptr; module = module->getParentModule()) {
        if (isApplication(module))
            return module;
    }
    return nullptr;
}

const cModule *PacketSourceBase::getContainingApplication() const
{
    auto application = findContainingApplication();
    if (application == nullptr)
        throw cRuntimeError("Application not found");
    else
        return application;
}

} // namespace queueing
} // namespace inet

