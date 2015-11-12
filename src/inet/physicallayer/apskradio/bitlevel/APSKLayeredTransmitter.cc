//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/analogmodel/bitlevel/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKLayeredTransmitter.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKEncoder.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKModulator.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrameSerializer.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKLayeredTransmitter);

APSKLayeredTransmitter::APSKLayeredTransmitter() :
    levelOfDetail((LevelOfDetail) - 1),
    encoder(nullptr),
    modulator(nullptr),
    pulseShaper(nullptr),
    digitalAnalogConverter(nullptr),
    power(W(NaN)),
    bitrate(bps(NaN)),
    bandwidth(Hz(NaN)),
    carrierFrequency(Hz(NaN))
{
}

void APSKLayeredTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        encoder = dynamic_cast<const IEncoder *>(getSubmodule("encoder"));
        modulator = dynamic_cast<const IModulator *>(getSubmodule("modulator"));
        pulseShaper = dynamic_cast<const IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = dynamic_cast<const IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        power = W(par("power"));
        bitrate = bps(par("bitrate"));
        bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("packet", levelOfDetailStr) == 0)
            levelOfDetail = PACKET_DOMAIN;
        else if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
        if (levelOfDetail >= BIT_DOMAIN && !encoder)
            throw cRuntimeError("Encoder not configured");
        if (levelOfDetail >= SYMBOL_DOMAIN && !modulator)
            throw cRuntimeError("Modulator not configured");
        if (levelOfDetail >= SAMPLE_DOMAIN && !pulseShaper)
            throw cRuntimeError("Pulse shaper not configured");
    }
}

std::ostream& APSKLayeredTransmitter::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKLayeredTransmitter";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", levelOfDetail = " << levelOfDetail
               << ", carrierFrequency = " << carrierFrequency;
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", encoder = " << printObjectToString(encoder, level - 1) 
               << ", modulator = " << printObjectToString(modulator, level - 1) 
               << ", pulseShaper = " << printObjectToString(pulseShaper, level - 1) 
               << ", digitalAnalogConverter = " << printObjectToString(digitalAnalogConverter, level - 1) 
               << ", power = " << power
               << ", bitrate = " << bitrate
               << ", bandwidth = " << bandwidth;
    return stream;
}

int APSKLayeredTransmitter::computePaddingLength(BitVector *bits) const
{
    const ConvolutionalCode *forwardErrorCorrection = nullptr;
    if (encoder) {
        const APSKEncoder *apskEncoder = check_and_cast<const APSKEncoder *>(encoder);
        forwardErrorCorrection = apskEncoder->getCode()->getConvolutionalCode();
    }
    int modulationCodeWordSize = check_and_cast<const APSKModulationBase *>(modulator->getModulation())->getCodeWordSize();
    int encodedCodeWordSize = forwardErrorCorrection == nullptr ? modulationCodeWordSize : modulationCodeWordSize *forwardErrorCorrection->getCodeRatePuncturingK();
    return (encodedCodeWordSize - bits->getSize() % encodedCodeWordSize) % encodedCodeWordSize;
}

const APSKPhyFrame *APSKLayeredTransmitter::createPhyFrame(const cPacket *macFrame) const
{
    APSKPhyFrame *phyFrame = new APSKPhyFrame();
    phyFrame->setByteLength(APSK_PHY_FRAME_HEADER_BYTE_LENGTH);
    phyFrame->encapsulate(const_cast<cPacket *>(macFrame));
    return phyFrame;
}

const ITransmissionPacketModel *APSKLayeredTransmitter::createPacketModel(const APSKPhyFrame *phyFrame) const
{
    if (levelOfDetail >= PACKET_DOMAIN) {
        BitVector *bits = APSKPhyFrameSerializer().serialize(phyFrame);
        bits->appendBit(0, computePaddingLength(bits));
        return new TransmissionPacketModel(phyFrame, bits, bitrate);
    }
    else
        return new TransmissionPacketModel(phyFrame, nullptr, bitrate);
}

const ITransmissionBitModel *APSKLayeredTransmitter::createBitModel(const ITransmissionPacketModel *packetModel) const
{
    if (levelOfDetail >= BIT_DOMAIN)
        return encoder->encode(packetModel);
    else {
        int netHeaderBitLength = APSK_PHY_FRAME_HEADER_BYTE_LENGTH * 8;
        int netPayloadBitLength = packetModel->getSerializedPacket()->getSize() - netHeaderBitLength;
        if (encoder) {
            const APSKEncoder *apskEncoder = check_and_cast<const APSKEncoder *>(encoder);
            const ConvolutionalCode *forwardErrorCorrection = apskEncoder->getCode()->getConvolutionalCode();
            if (forwardErrorCorrection == nullptr)
                return new TransmissionBitModel(netHeaderBitLength, bitrate, netPayloadBitLength, bitrate, nullptr, forwardErrorCorrection, nullptr, nullptr);
            else {
                int grossHeaderBitLength = forwardErrorCorrection->getEncodedLength(netHeaderBitLength);
                int grossPayloadBitLength = forwardErrorCorrection->getEncodedLength(netPayloadBitLength);
                bps grossBitrate = bitrate / forwardErrorCorrection->getCodeRate();
                return new TransmissionBitModel(grossHeaderBitLength, grossBitrate, grossPayloadBitLength, grossBitrate, nullptr, forwardErrorCorrection, nullptr, nullptr);
            }
        }
        else
            return new TransmissionBitModel(netHeaderBitLength, bitrate, netPayloadBitLength, bitrate, nullptr, nullptr, nullptr, nullptr);
    }
}

const ITransmissionSymbolModel *APSKLayeredTransmitter::createSymbolModel(const ITransmissionBitModel *bitModel) const
{
    if (levelOfDetail >= SYMBOL_DOMAIN)
        return modulator->modulate(bitModel);
    else
        return new TransmissionSymbolModel(-1, NaN, -1, NaN, nullptr, modulator->getModulation(), modulator->getModulation());
}

const ITransmissionSampleModel *APSKLayeredTransmitter::createSampleModel(const ITransmissionSymbolModel *symbolModel) const
{
    if (levelOfDetail >= SAMPLE_DOMAIN)
        return pulseShaper->shape(symbolModel);
    else
        return nullptr;
}

const ITransmissionAnalogModel *APSKLayeredTransmitter::createAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSampleModel *sampleModel) const
{
    if (digitalAnalogConverter) {
        if (sampleModel == nullptr)
            throw cRuntimeError("Digital analog converter needs sample domain representation");
        else
            return digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
    }
    else {
        simtime_t duration = packetModel->getPacket()->getBitLength() / bitrate.get();
        return new ScalarTransmissionSignalAnalogModel(duration, carrierFrequency, bandwidth, power);
    }
}

const ITransmission *APSKLayeredTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const APSKPhyFrame *phyFrame = createPhyFrame(macFrame);
    const ITransmissionPacketModel *packetModel = createPacketModel(phyFrame);
    const ITransmissionBitModel *bitModel = createBitModel(packetModel);
    const ITransmissionSymbolModel *symbolModel = createSymbolModel(bitModel);
    const ITransmissionSampleModel *sampleModel = createSampleModel(symbolModel);
    const ITransmissionAnalogModel *analogModel = createAnalogModel(packetModel, bitModel, sampleModel);
    // assuming movement and rotation during transmission is negligible
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, -1, -1, -1, startPosition, endPosition, startOrientation, endOrientation);
}

} // namespace physicallayer

} // namespace inet

