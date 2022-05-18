//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/packetlevel/ApskRadio.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/physicallayer/wireless/apsk/bitlevel/ApskEncoder.h"
#include "inet/physicallayer/wireless/apsk/bitlevel/ApskLayeredTransmitter.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {
namespace physicallayer {

Define_Module(ApskRadio);

ApskRadio::ApskRadio() :
    FlatRadioBase()
{
}

void ApskRadio::initialize(int stage)
{
    Radio::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *protocolName = par("protocol");
        if (*protocolName != '\0')
            protocol = Protocol::getProtocol(protocolName);
    }
}

void ApskRadio::handleUpperPacket(Packet *packet)
{
    if (protocol != nullptr && protocol != packet->getTag<PacketProtocolTag>()->getProtocol())
        throw cRuntimeError("Packet received with incorrect protocol");
    else
        FlatRadioBase::handleUpperPacket(packet);
}

void ApskRadio::sendUp(Packet *packet)
{
    if (protocol != nullptr && protocol != packet->getTag<PacketProtocolTag>()->getProtocol()) {
        PacketDropDetails details;
        details.setReason(PacketDropReason::OTHER_PACKET_DROP);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
    else
        FlatRadioBase::sendUp(packet);
}

b ApskRadio::computePaddingLength(b length, const ConvolutionalCode *forwardErrorCorrection, const ApskModulationBase *modulation) const
{
    int modulationCodeWordSize = modulation->getCodeWordSize();
    int encodedCodeWordSize = forwardErrorCorrection == nullptr ? modulationCodeWordSize : modulationCodeWordSize *forwardErrorCorrection->getCodeRatePuncturingK();
    return b((encodedCodeWordSize - b(length).get() % encodedCodeWordSize) % encodedCodeWordSize);
}

const ApskModulationBase *ApskRadio::getModulation() const
{
    const ApskModulationBase *modulation = nullptr;
//    const ConvolutionalCode *forwardErrorCorrection = nullptr; // TODO
    auto phyHeader = makeShared<ApskPhyHeader>();
    b headerLength = phyHeader->getChunkLength();

    // KLUDGE
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        headerLength = flatTransmitter->getHeaderLength();
        modulation = check_and_cast<const ApskModulationBase *>(flatTransmitter->getModulation());
    }
    // KLUDGE
    else if (auto layeredTransmitter = dynamic_cast<const ApskLayeredTransmitter *>(transmitter)) {
        auto encoder = layeredTransmitter->getEncoder();
        if (encoder != nullptr) {
//            const ApskEncoder *apskEncoder = check_and_cast<const ApskEncoder *>(encoder);
//            forwardErrorCorrection = apskEncoder->getCode()->getConvolutionalCode(); // TODO
        }
        modulation = check_and_cast<const ApskModulationBase *>(layeredTransmitter->getModulator()->getModulation());
    }
    // FIXME when uses OFDM, ofdm modulator can not cast to apsk modulator, see /examples/wireless/layered80211/ -f omnetpp.ini -c LayeredCompliant80211Ping
    ASSERT(modulation != nullptr);
    return modulation;
}

void ApskRadio::encapsulate(Packet *packet) const
{
    auto phyHeader = makeShared<ApskPhyHeader>();
    phyHeader->setCrc(0);
    phyHeader->setCrcMode(CRC_DISABLED);
    phyHeader->setPayloadLengthField(packet->getDataLength());
    phyHeader->setPayloadProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    b headerLength = phyHeader->getChunkLength();
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        headerLength = flatTransmitter->getHeaderLength();
        phyHeader->setChunkLength(headerLength);
    }
    phyHeader->setHeaderLengthField(headerLength);
    packet->insertAtFront(phyHeader);

    auto paddingLength = computePaddingLength(headerLength + phyHeader->getPayloadLengthField(), nullptr, getModulation());
    if (paddingLength != b(0))
        packet->insertAtBack(makeShared<BitCountChunk>(paddingLength));
    EV_DEBUG << "ApskRadio::encapsulate: packetLength=" << packet->getDataLength() << ", headerLength=" << headerLength << ", paddingLength=" << paddingLength << endl;
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::apskPhy);
}

void ApskRadio::decapsulate(Packet *packet) const
{
    const auto& phyHeader = packet->popAtFront<ApskPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT | Chunk::PF_ALLOW_INCOMPLETE | Chunk::PF_ALLOW_IMPROPERLY_REPRESENTED);
    if (phyHeader->isIncorrect() || phyHeader->isIncomplete() || phyHeader->isImproperlyRepresented())
        packet->setBitError(true);
    b headerLength = phyHeader->getChunkLength();

#if 0
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        b definedHeaderLength = flatTransmitter->getHeaderLength();
        if (headerLength != definedHeaderLength)
            throw cRuntimeError("Incoming header length is incorrect");
    }
#endif

    auto paddingLength = computePaddingLength(headerLength + phyHeader->getPayloadLengthField(), nullptr, getModulation());
    if (paddingLength > b(0)) {
        if (paddingLength <= packet->getDataLength())
            packet->popAtBack(paddingLength, Chunk::PF_ALLOW_INCORRECT);
        else
            packet->setBitError(true);
    }

    // KLUDGE? higher layers accepts only byte length packets started on byte position
    if (packet->getBitLength() % 8 != 0 || headerLength.get() % 8 != 0)
        packet->setBitError(true);

    if (phyHeader->getPayloadLengthField() > packet->getDataLength())
        packet->setBitError(true);

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(phyHeader->getPayloadProtocol());
}

} // namespace physicallayer
} // namespace inet

