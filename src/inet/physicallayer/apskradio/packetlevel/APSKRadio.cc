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

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKEncoder.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKLayeredTransmitter.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKPhyHeader_m.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKRadio.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKRadio);

APSKRadio::APSKRadio() :
    FlatRadioBase()
{
}

int APSKRadio::computePaddingLength(int64_t bitLength, const ConvolutionalCode *forwardErrorCorrection, const APSKModulationBase *modulation) const
{
    int modulationCodeWordSize = modulation->getCodeWordSize();
    int encodedCodeWordSize = forwardErrorCorrection == nullptr ? modulationCodeWordSize : modulationCodeWordSize * forwardErrorCorrection->getCodeRatePuncturingK();
    return (encodedCodeWordSize - bitLength % encodedCodeWordSize) % encodedCodeWordSize;
}

// TODO: split into APSKLayeredRadio
void APSKRadio::encapsulate(Packet *packet) const
{
    const APSKModulationBase *modulation = nullptr;
    const ConvolutionalCode *forwardErrorCorrection = nullptr;
    auto phyHeader = std::make_shared<APSKPhyHeader>();

    // KLUDGE:
    if (auto flatTransmitter = dynamic_cast<const FlatTransmitterBase *>(transmitter)) {
        phyHeader->setChunkLength(flatTransmitter->getHeaderLength());
        modulation = check_and_cast<const APSKModulationBase *>(flatTransmitter->getModulation());
    }
    // KLUDGE:
    else if (auto layeredTransmitter = dynamic_cast<const APSKLayeredTransmitter *>(transmitter)) {
        auto encoder = layeredTransmitter->getEncoder();
        if (encoder != nullptr) {
            const APSKEncoder *apskEncoder = check_and_cast<const APSKEncoder *>(encoder);
            forwardErrorCorrection = apskEncoder->getCode()->getConvolutionalCode();
        }
        modulation = check_and_cast<const APSKModulationBase *>(layeredTransmitter->getModulator()->getModulation());
    }
    // KLUDGE:
    else {
        phyHeader->setChunkLength(byte(1));
        ASSERT(modulation != nullptr);  //FIXME when uses OFDM, ofdm modulator can not cast to apsk modulator, see /examples/wireless/layered80211/ -f omnetpp.ini -c LayeredCompliant80211Ping
    }

    ASSERT(modulation != nullptr);

    phyHeader->setCrc(0);
    phyHeader->setCrcMode(CRC_DISABLED);
    phyHeader->setLengthField(packet->getByteLength());
    phyHeader->markImmutable();
    packet->pushHeader(phyHeader);
    auto paddingBitLength = bit(computePaddingLength(bit(packet->getTotalLength()).get(), nullptr, modulation));
    if (paddingBitLength != bit(0)) {
        auto paddingTrailer = std::make_shared<ByteCountChunk>(paddingBitLength);
        paddingTrailer->markImmutable();
        packet->pushTrailer(paddingTrailer);
    }
}

void APSKRadio::decapsulate(Packet *packet) const
{
    packet->popHeader<APSKPhyHeader>();
}

} // namespace physicallayer

} // namespace inet

