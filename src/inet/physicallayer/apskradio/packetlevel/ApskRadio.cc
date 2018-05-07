//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskEncoder.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskLayeredTransmitter.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskRadio.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskRadio);

ApskRadio::ApskRadio() :
    FlatRadioBase()
{
}

b ApskRadio::computePaddingLength(b length, const ConvolutionalCode *forwardErrorCorrection, const ApskModulationBase *modulation) const
{
    int modulationCodeWordSize = modulation->getCodeWordSize();
    int encodedCodeWordSize = forwardErrorCorrection == nullptr ? modulationCodeWordSize : modulationCodeWordSize * forwardErrorCorrection->getCodeRatePuncturingK();
    return b((encodedCodeWordSize - b(length).get() % encodedCodeWordSize) % encodedCodeWordSize);
}

const ApskModulationBase *ApskRadio::getModulation() const
{
    const ApskModulationBase *modulation = nullptr;
    // TODO: const ConvolutionalCode *forwardErrorCorrection = nullptr;
    auto phyHeader = makeShared<ApskPhyHeader>();
    b headerLength = phyHeader->getChunkLength();

    // KLUDGE:
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        headerLength = flatTransmitter->getHeaderLength();
        modulation = check_and_cast<const ApskModulationBase *>(flatTransmitter->getModulation());
    }
    // KLUDGE:
    else if (auto layeredTransmitter = dynamic_cast<const ApskLayeredTransmitter *>(transmitter)) {
        auto encoder = layeredTransmitter->getEncoder();
        if (encoder != nullptr) {
            // const ApskEncoder *apskEncoder = check_and_cast<const ApskEncoder *>(encoder);
            // TODO: forwardErrorCorrection = apskEncoder->getCode()->getConvolutionalCode();
        }
        modulation = check_and_cast<const ApskModulationBase *>(layeredTransmitter->getModulator()->getModulation());
    }
    //FIXME when uses OFDM, ofdm modulator can not cast to apsk modulator, see /examples/wireless/layered80211/ -f omnetpp.ini -c LayeredCompliant80211Ping
    ASSERT(modulation != nullptr);
    return modulation;
}

void ApskRadio::encapsulate(Packet *packet) const
{
    auto phyHeader = makeShared<ApskPhyHeader>();
    phyHeader->setCrc(0);
    phyHeader->setCrcMode(CRC_DISABLED);
    phyHeader->setLengthField(packet->getByteLength());
    phyHeader->setPayloadProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    b headerLength = phyHeader->getChunkLength();
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        headerLength = flatTransmitter->getHeaderLength();
        if (headerLength > phyHeader->getChunkLength())
            packet->insertAtFront(makeShared<BitCountChunk>(headerLength - phyHeader->getChunkLength()));
    }
    packet->insertAtFront(phyHeader);
    auto paddingLength = computePaddingLength(headerLength + B(phyHeader->getLengthField()), nullptr, getModulation());
    if (paddingLength != b(0))
        packet->insertAtBack(makeShared<BitCountChunk>(paddingLength));
    packet->getTag<PacketProtocolTag>()->setProtocol(&Protocol::apskPhy);
}

void ApskRadio::decapsulate(Packet *packet) const
{
    const auto& phyHeader = packet->popAtFront<ApskPhyHeader>(b(-1), Chunk::PF_ALLOW_INCORRECT);
    b headerLength = phyHeader->getChunkLength();
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        headerLength = flatTransmitter->getHeaderLength();
        if (headerLength > phyHeader->getChunkLength())
            packet->popAtFront(headerLength - phyHeader->getChunkLength(), Chunk::PF_ALLOW_INCORRECT);
    }
    auto paddingLength = computePaddingLength(headerLength + B(phyHeader->getLengthField()), nullptr, getModulation());
    if (paddingLength != b(0))
        packet->popAtBack(paddingLength, Chunk::PF_ALLOW_INCORRECT);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(phyHeader->getPayloadProtocol());
}

} // namespace physicallayer

} // namespace inet

